using System;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using System.Threading;

namespace MonoUtils
{
    class SafeSerialPort : SerialPort
    {
        private readonly bool _isRunningOnMono;
        private Timer _receiveSerial;
        private string _portName;

        public SafeSerialPort()
        {
            base.DataReceived += _serial_DataReceived;
            _isRunningOnMono = Utilities.IsRunningOnMono();
        }

        public void BeginReceivingData()
        {
            // If is running mono, then set a manual check
            if (_isRunningOnMono)
                _receiveSerial = new Timer(ManualSerial, null, 0, 100);
        }

        private void ManualSerial(object state)
        {
            try
            {
                if (!IsOpen || BytesToRead <= 0) return;

                var constructor = typeof(SerialDataReceivedEventArgs).GetConstructor(
                    BindingFlags.NonPublic | BindingFlags.Instance, null, new[] { typeof(SerialData) }, null);

                var eventArgs = (SerialDataReceivedEventArgs)constructor.Invoke(new object[] { SerialData.Chars });

                OnDataReceived(eventArgs);
            }
            catch { }
        }

        void _serial_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            OnDataReceived(e);
        }
        /*

        private void sensors_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (!_sensorIsActive)
                return;

            const string prefix = "$$";
            const char separator = ':';
            
            string inbuff = ((SerialPort)sender).ReadExisting();
            if (inbuff.StartsWith(prefix))
                _instring2 = inbuff;
            else
                _instring2 += inbuff;

            _arduinoString = _instring2.Split();
            foreach (var buffer in _arduinoString)
            {
                // Parse arduino states
                if (buffer.StartsWith(prefix))
                {
                    if (buffer.Contains(separator))
                    {
                        var sensor = buffer.Substring(prefix.Length).Split(new[] { separator });

                        if (sensor.Count() >= 2)
                        {
                            if (ignoreEmpty && sensor[1].Length == 0)
                                continue;

                            var s = "{" + sensor[0] + "}";
                            if (!_analogs.ContainsKey(s))
                                _analogs.Add(s, sensor[1]);
                            else
                                _analogs[s] = sensor[1];
                        }
                    }
                }
            }
        }

        private static void Log(String details)
        {
            try
            {
                string e = DateTime.Now.ToShortDateString() + " " + DateTime.Now.ToLongTimeString() + ": " + details;
                File.AppendAllText("error.log", e + Environment.NewLine);
            }
            catch
            {
            }
        }

        private void keepPortsAlive(object state)
        {
            if (SafeIsOpen(_sensors)) return;

            if (!SafeIsOpen(_sensors))
            {
                try
                {
                    _sensors.Close();
                }
                catch
                {
                }

                try
                {
                    SafeOpen(_sensors);
                }
                catch
                {
                }
            }

            _sensorIsActive = SafeIsOpen(_sensors);
        }

        private void ManualSerial(object state)
        {
            sensors_DataReceived(_sensors, null);
        }*/

        private bool SafeIsOpen()
        {
            if (String.IsNullOrEmpty(PortName))
                return false;

            if (_isRunningOnMono)
                return File.Exists(PortName) && base.IsOpen;

            return base.IsOpen;
        }

        private void SafeOpen()
        {
            if (String.IsNullOrEmpty(PortName))
                return;

            var tryToOpen = true;

            if (_isRunningOnMono)
                tryToOpen = File.Exists(PortName);

            if (tryToOpen)
                base.Open();
        }

        public new bool IsOpen
        {
            get { return SafeIsOpen(); }
        }

        internal new void Open()
        {
            SafeOpen();
        }

        public new event EventHandler<SerialDataReceivedEventArgs> DataReceived;

        protected virtual void OnDataReceived(SerialDataReceivedEventArgs e)
        {
            DataReceived?.Invoke(this, e);
        }
    }

}