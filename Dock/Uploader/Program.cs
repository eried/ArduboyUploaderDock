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
        private const string RepoDirectory = "repo";
        private static string _buffer;
        static object receiving = new object();

        private static void Main(string[] args)
        {
            Environment.CurrentDirectory = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            LoadGamesFromRepo();

            Log("Started. Waiting for incoming device.");

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
                                    if (TimeSinceLastPingReceived.ElapsedMilliseconds > 1000)
                                    {
                                        Log("Not responding to PING...");
                                        SendHexToArduboyReset(s);
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

                //Log("Nothing connected. Retrying soon");

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

        private static void LoadGamesFromRepo()
        {
            hexFiles = Directory.GetFiles(RepoDirectory, "*.hex", SearchOption.AllDirectories);
            Log("Loaded: " + hexFiles.Count() + " hex files");
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
            lock (receiving)
            {
                var s = (SerialPort)sender;
                _buffer += s.ReadExisting().Replace("\r", "").Replace("\n", "");

                if (_buffer.Contains(CommandEnd))
                {
                    var tmp = _buffer.Split(CommandEnd);
                    var includeFinalItem = _buffer[_buffer.Length - 1] == CommandEnd;
                    for (var i = 0; i < tmp.Length - (includeFinalItem ? 0 : 1); i++)
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
        }

        public static double ConvertToUnixTimestamp(DateTime date)
        {
            DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Local);
            TimeSpan diff = date.ToLocalTime() - origin;
            return Math.Floor(diff.TotalSeconds);
        }

        private static void ProcessCommand(IReadOnlyList<string> cmd, SerialPort s)
        {
            var considerCommandReceivedAsPing = true;

            try
            {
                Ping(s);

                switch (cmd[0])
                {
                    case "PING":
                    case "PONG":
                        break;

                    case "REPOSIZE":
                        Log("REPO SIZE received");
                        SendResponseToArduboy(s, hexFiles.Length + "");
                        break;

                    case "REPOSEND":
                        {
                            considerCommandReceivedAsPing = false;

                            int n;
                            if (cmd.Count >= 2 && int.TryParse(cmd[1], out n))
                            {
                                Log("REPO SEND received: " + n);
                                var g = GetRepoHex(n);
                                if (g != null)
                                    SendHexToArduboyReset(s, g.Hex);
                            }
                        }
                        break;

                    case "REPONAME":
                        {
                            var response = "NOT FOUND";
                            int n;
                            if (cmd.Count >= 2 && int.TryParse(cmd[1], out n))
                            {
                                Log("REPO NAME received: " + n);
                                var g = GetRepoHex(n);
                                response = g == null ? "OUT OF BOUNDS" : g.Category.PadRight(6).Substring(0,6) +"/"+ g.Name.PadLeft(17).Substring(0, 17);
                            }
                            SendResponseToArduboy(s, response);
                        }
                        break;

                    case "UPDATE":
                        Log("UPDATE received");
                        SendHexToArduboyReset(s);
                        Log("Uploader sent. Waiting");
                        break;

                    case "SEND":
                        Log("SEND received");
                        considerCommandReceivedAsPing = false;
                        SendHexToArduboyReset(s, cmd[1]);
                        break;

                    case "TIME":
                        Log("TIME received");
                        SendResponseToArduboy(s, ((long)ConvertToUnixTimestamp(DateTime.Now)) + "");
                        break;

                    case "ABOUT":
                        Log("ABOUT received");
                        break;

                    case "REPOUPDATE":
                        Log("REPOUPDATE received");
                        new Thread(new ThreadStart(OnlineRepoUpdate)).Start();
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
            }
            catch (Exception ex) { Log("Error in command: " + ex.Message); }

            if (considerCommandReceivedAsPing)
                TimeSinceLastPingReceived.Restart();
        }

        private static void OnlineRepoUpdate()
        {
            Process.Start(new ProcessStartInfo() { FileName = "git", Arguments = "reset --hard HEAD", WorkingDirectory = RepoDirectory }).WaitForExit();
            Process.Start(new ProcessStartInfo() { FileName = "git", Arguments = "clean -xffd", WorkingDirectory = RepoDirectory }).WaitForExit();
            Process.Start(new ProcessStartInfo() { FileName = "git", Arguments = "pull", WorkingDirectory = RepoDirectory }).WaitForExit();
            LoadGamesFromRepo();
        }

        private static RepoItem GetRepoHex(int n)
        {
            if (n < hexFiles.Length && n >= 0)
            {
                var hex = hexFiles[n];
                var gamePath = Path.GetDirectoryName(hex);
                var game = Path.GetFileName(gamePath);
                var category = Path.GetFileName(Path.GetDirectoryName(gamePath));
                return new RepoItem { Category = category, Name = game, Hex = hex };
            }
            return null;
        }

        private static void SendHexToArduboyReset(SerialPort s, string hex = null)
        {
            var findDockHex = string.IsNullOrEmpty(hex);
            s.Close();
            TimeSinceLastPingReceived.Reset();

            if (findDockHex)
                hex = GetDockHex();

            // If we are in Windows, we could use Arduboy Uploader for this...
            const string abuploader = "abupload.exe";
            if (!Utilities.IsRunningOnMono() && File.Exists(abuploader))
                Process.Start(new ProcessStartInfo { FileName = abuploader, Arguments = "\""+ Path.GetFullPath(hex)+ "\"" }).WaitForExit();
            else
            {
                ResetAndWait();
                TransferHexToArduboy(hex);
            }
            _waitForTheDeviceToBeDisconnected = !findDockHex;
            Log("Game: " + hex + " sent. Waiting");
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

        private static void TransferHexToArduboy(string hexFile)
        {
            if (string.IsNullOrEmpty(hexFile) || !File.Exists(hexFile))
                return;

            var p = new Process { StartInfo = new ProcessStartInfo("avrdude",
                $"-C/etc/avrdude.conf -p atmega32u4 -V -q -c avr109 -P {GetFirstArduboy()} -b 115200 -D -U flash:w:\"{Path.GetFullPath(hexFile)}\":i") };

            p.Start();

            p.WaitForExit(10000);
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
        private static string[] hexFiles;

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

    internal class RepoItem
    {
        internal string Category;
        internal string Name;
        internal string Hex;
    }
}
