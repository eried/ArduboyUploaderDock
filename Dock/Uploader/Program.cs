using MonoUtils;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Uploader
{
    class Program
    {
        static Stopwatch lastConnectedArduboy = new Stopwatch();
        private static bool _waitDisconnect;

        static void Main(string[] args)
        {
            Environment.CurrentDirectory = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);

            do
            {
                try
                {
                    var arduboy = GetFirstArduboy();

                    if (!string.IsNullOrEmpty(arduboy))
                    {
                        var s = new SafeSerialPort()
                        {
                            BaudRate = 115200,
                            PortName = arduboy,
                            RtsEnable = true,
                        };

                        s.Open();
                        s.BeginReceivingData();
                        s.DataReceived += S_DataReceived;
                        Log("Connected to " + s.PortName);
                        lastConnectedArduboy.Restart();
                        s.WriteLine("PING");

                        do
                        {
                            try
                            {
                                if (!s.IsOpen)
                                {
                                    Log("Lost connection...");
                                    s.Close();
                                    Thread.Sleep(1000);
                                    s.Open();

                                    if (!s.IsOpen)
                                        break;
                                }
                                else
                                {
                                    if (lastConnectedArduboy.ElapsedMilliseconds > 3000)
                                    {
                                        Log("Not responding to PING...");
                                        SendUploader(s);
                                    }
                                }
                            }
                            catch (Exception ex)
                            {
                                Log("Error in loop: " + ex.Message);
                                break;
                            }

                            Thread.Sleep(100);

                        } while (true);
                    }
                }
                catch (Exception ex)
                {
                    Log("Error: " + ex.Message);
                }

                Log("Nothing connected. Retrying soon");

                if (_waitDisconnect)
                {
                    Log("Waiting");
                    Thread.Sleep(4000);
                    Log("... for disconnection.");
                    WaitForTheArduboy();
                    _waitDisconnect = false;
                }
                else
                    Thread.Sleep(1000);
            } while (true);
        }

        private static void S_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            var sp = (SerialPort)sender;

            var cmd = sp.ReadExisting().Replace("\r", "").Replace("\n", "").Trim().Split(':');
            switch (cmd[0])
            {
                case "PING":
                case "PONG":
                    Log("PING/PONG received");
                    sp.WriteLine("PING");
                    lastConnectedArduboy.Restart();
                    break;

                case "REPO":
                    Log("REPO received");

                    foreach(var hex in Directory.GetFiles("repo","*.hex", SearchOption.AllDirectories))
                    {
                        var gamePath = Path.GetDirectoryName(hex);
                        var game = Path.GetFileName(gamePath);
                        var category = Path.GetFileName(Path.GetDirectoryName(gamePath));

                        sp.Write(category.Substring(0, Math.Min(3, category.Length)) + "/" + game.Substring(0, Math.Min(6, game.Length)) + " ");
                    }

                    lastConnectedArduboy.Restart();
                    break;

                case "UPDATE":
                    Log("UPDATE received");
                    SendUploader(sp);
                    Log("Uploader sent. Waiting");
                    break;

                case "SEND":
                    Log("SEND received");
                    sp.Close();
                    lastConnectedArduboy.Reset();
                    ResetAndWait();
                    SendArduboyGame(cmd[1],true);
                    Log("Game sent. Waiting");
                    break;

                case "TIME":
                    Log("TIME received");
                    sp.WriteLine(DateTime.Now.ToShortDateString());
                    sp.WriteLine(DateTime.Now.ToShortTimeString());
                    break;

                case "ABOUT":
                    Log("ABOUT received");
                    sp.WriteLine("Arduboy Uploader Dock");
                    sp.WriteLine("v0.1 20170712" + Environment.NewLine);
                    sp.WriteLine("Erwin Ried");

                    break;

                case "SHUTDOWN":
                    lastConnectedArduboy.Reset();
                    Log("SHUTDOWN received");
                    Process.Start(new ProcessStartInfo() { FileName = "sudo", Arguments = "halt" });
                    break;

                default:
                    Log("Received: " + cmd[0]);
                    break;
            }
        }

        private static void SendUploader(SerialPort sp)
        {
            sp.Close();
            lastConnectedArduboy.Reset();
            ResetAndWait();
            SendArduboyGame(GetDockHex(), false);
        }

        private static string GetDockHex()
        {
            foreach (var f in Directory.EnumerateFiles(".", "*.hex"))
                if (Path.GetFileNameWithoutExtension(f).ToLower().Contains("uploader"))
                    return f;
            return null;
        }

        private static void ResetAndWait()
        {
            SendReset(GetFirstArduboy());
            
            WaitForTheArduboy();
        }

        private static void WaitForTheArduboy(bool fast = false)
        {
            if(!fast)
                while (!string.IsNullOrEmpty(GetFirstArduboy())) Thread.Sleep(100);
            while (string.IsNullOrEmpty(GetFirstArduboy())) Thread.Sleep(100);
        }

        private static void SendArduboyGame(string hexFile, bool waitForDisconnection = true)
        {
            if (string.IsNullOrEmpty(hexFile) || !File.Exists(hexFile))
                return;

            var p = new Process { StartInfo = new ProcessStartInfo("avrdude",
                $"-C/etc/avrdude.conf -p atmega32u4 -V -q -c avr109 -P {GetFirstArduboy()} -b 115200 -D -U flash:w:\"{Path.GetFullPath(hexFile)}\":i") };

            p.Start();

            p.WaitForExit(10000);

            _waitDisconnect = waitForDisconnection;
            Log("Sent!");
        }

        private static string GetFirstArduboy()
        {
            try
            {
                foreach (var d in Directory.GetFiles("/dev/serial/by-id/"))
                    return d;
            }
            catch { }
            return null;
        }

        private static void Log(string msg)
        {
            Console.WriteLine(msg);
        }


        /// <summary>
        /// Send reset signal via opening and closing the port at 1200 bauds
        /// </summary>
        /// <param name="port">COM port</param>
        private static void SendReset(string port)
        {
            var arduboy = new SafeSerialPort
            {
                BaudRate = 1200,
                PortName = port,
                DtrEnable = true
            };
            arduboy.Open();

            while (!arduboy.IsOpen)
                Thread.Sleep(100);

            arduboy.DtrEnable = false;
            arduboy.Close();
            arduboy.Dispose();
        }
    }
}
