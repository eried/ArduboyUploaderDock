using SafeSerialPort;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Threading;

namespace Uploader
{
    class Program
    {
        private static readonly Stopwatch TimeSinceLastPingReceived = Stopwatch.StartNew(), 
            TimeSinceLastPingSent = Stopwatch.StartNew();
        private static bool _waitForTheDeviceToBeDisconnected;
        private const char CommandEnd = '>', CommandStart = '<', CommandSplit = ':';
        private static string _buffer;

        private static void Main(string[] args)
        {
            Environment.CurrentDirectory = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);

            do
            {
                try
                {
                    var arduboy = GetFirstArduboy();

                    if (!string.IsNullOrEmpty(arduboy))
                    {
                        var s = ConnectToArduboy(arduboy);

                        do
                        {
                            try
                            {
                                if (!s.IsOpen)
                                {
                                    Log("Lost connection...");
                                    s.Close();
                                    Thread.Sleep(500);
                                    s.Open();

                                    if (!s.IsOpen)
                                        break;
                                }
                                else
                                {
                                    if (TimeSinceLastPingReceived.ElapsedMilliseconds > 2000)
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

                            Thread.Sleep(10);

                        } while (true);
                    }
                }
                catch (Exception ex)
                {
                    Log("Error: " + ex.Message);
                }

                Log("Nothing connected. Retrying soon");

                if (_waitForTheDeviceToBeDisconnected)
                {
                    Log("Waiting");
                    Thread.Sleep(3000);
                    Log("... for disconnection.");
                    WaitForTheArduboy();
                    _waitForTheDeviceToBeDisconnected = false;
                }
                else
                    Thread.Sleep(200);
            } while (true);
        }

        private static SafeSerialPort ConnectToArduboy(string arduboy)
        {
            var s = new SafeSerialPort()
            {
                BaudRate = 115200,
                PortName = arduboy,
                RtsEnable = true,
            };

            s.Open(true);
            s.DataReceived += S_DataReceived;
            Log("Connected to " + s.PortName);
            TimeSinceLastPingReceived.Restart();
            Ping(s); // Initial ping
            return s;
        }

        private static void Ping(SerialPort s)
        {
            if (TimeSinceLastPingSent.ElapsedMilliseconds <= 100) return;

            SendResponseToArduboy(s,"PING");
            TimeSinceLastPingSent.Restart();
        }

        private static void SendResponseToArduboy(SerialPort s, string cmd)
        {
            if(s.IsOpen)
                s.Write(CommandStart + cmd + CommandEnd);
        }

        private static void S_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            var s = (SerialPort)sender;
            _buffer += s.ReadExisting().Replace("\r", "").Replace("\n", "");

            if (_buffer.Contains(CommandEnd))
            {
                var tmp = _buffer.Split(CommandEnd);
                var includeFinalItem = _buffer[_buffer.Length - 1] == CommandEnd;
                for (var i = 0; i < tmp.Length-(includeFinalItem?0:1); i++)
                {
                    if (!string.IsNullOrEmpty(tmp[i]) && tmp[i][0] == CommandStart)
                    {
                        tmp[i] = tmp[i].TrimStart(CommandStart);
                        ProcessCommand(tmp[i].Split(CommandSplit), s);
                    }
                }

                _buffer = !includeFinalItem ? tmp.Last() : "";
            }      
        }

        public static double ConvertToUnixTimestamp(DateTime date)
        {
            DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Local);
            TimeSpan diff = date.ToLocalTime() - origin;
            return Math.Floor(diff.TotalSeconds);
        }

        private static void ProcessCommand(IReadOnlyList<string> cmd, SerialPort s)
        {
            Ping(s);
            var considerCommandReceivedAsPing = true;

            switch (cmd[0])
            {
                case "PING":
                case "PONG":
                    /*Log("PING/PONG received");
                    Ping(s);
                    TimeSinceLastPingReceived.Restart();*/
                    break;

                case "REPOSIZE":
                    Log("REPO SIZE received");
                    SendResponseToArduboy(s, Directory.GetFiles("repo", "*.hex", SearchOption.AllDirectories).Length+"");
                    break;

                case "REPONAME":
                    Log("REPO NAME received");
                    var response = "NOT FOUND";
                    if (cmd.Count >= 2 && int.TryParse(cmd[1], out int n))
                    {
                        var games = Directory.GetFiles("repo", "*.hex", SearchOption.AllDirectories);

                        if (n < games.Length && n >= 0)
                        {
                            var hex = games[n];
                            var gamePath = Path.GetDirectoryName(hex);
                            var game = Path.GetFileName(gamePath);
                            var category = Path.GetFileName(Path.GetDirectoryName(gamePath));
                            response = game;
                        }
                        else
                            response = "OUT OF BOUNDS";
                    }
                    SendResponseToArduboy(s, response);
                    break;

                case "UPDATE":
                    Log("UPDATE received");
                    SendUploader(s);
                    Log("Uploader sent. Waiting");
                    break;

                case "SEND":
                    Log("SEND received");
                    s.Close();
                    considerCommandReceivedAsPing = false;
                    TimeSinceLastPingReceived.Reset();
                    ResetAndWait();
                    SendArduboyGame(cmd[1], true);
                    Log("Game sent. Waiting");
                    break;

                case "TIME":
                    Log("TIME received");
                    SendResponseToArduboy(s, ((long)ConvertToUnixTimestamp(DateTime.Now))+"");
                    break;

                case "ABOUT":
                    Log("ABOUT received");
                    break;

                case "SHUTDOWN":
                    TimeSinceLastPingReceived.Reset();
                    Log("SHUTDOWN received");
                    Process.Start(new ProcessStartInfo() { FileName = "sudo", Arguments = "halt" });
                    break;

                default:
                    Log("Received: " + cmd[0]);
                    considerCommandReceivedAsPing = false;
                    break;
            }

            if (considerCommandReceivedAsPing)
                TimeSinceLastPingReceived.Restart();
        }

        private static void SendUploader(SerialPort sp)
        {
            sp.Close();
            TimeSinceLastPingReceived.Reset();
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

            _waitForTheDeviceToBeDisconnected = waitForDisconnection;
            Log("Sent!");
        }

        private static string GetFirstArduboy()
        {
            try
            {
                foreach (var d in Utilities.IsRunningOnMono()? Directory.GetFiles("/dev/serial/by-id/"): SerialPort.GetPortNames())
                    return d;
            }
            catch { }
            return null;
        }

        static string _lastMsg = "";
        private static void Log(string msg)
        {
            if (msg != _lastMsg)
            {
                _lastMsg = msg;
                Console.WriteLine(msg);
            }
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
