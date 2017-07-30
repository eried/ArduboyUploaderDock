using System;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using System.Threading;
using SafeSerialPort;

namespace Uploader
{
    internal class SafeSerialPort : SerialPort
    {
        private readonly bool _isRunningOnMono;
        private Timer _receiveSerial;

        public SafeSerialPort()
        {
            base.DataReceived += _serial_DataReceived;
            _isRunningOnMono = Utilities.IsRunningOnMono();
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
       
        private bool SafeIsOpen()
        {
            if (string.IsNullOrEmpty(PortName))
                return false;

            if (_isRunningOnMono)
                return File.Exists(PortName) && base.IsOpen;

            return base.IsOpen;
        }

        private void SafeOpen()
        {
            if (string.IsNullOrEmpty(PortName))
                return;

            var tryToOpen = true;

            if (_isRunningOnMono)
                tryToOpen = File.Exists(PortName);

            if (tryToOpen)
                base.Open();
        }

        public new bool IsOpen => SafeIsOpen();

        internal new void Open()
        {
            Open(false);
        }

        internal new void Close()
        {
            _receiveSerial?.Dispose();
            _receiveSerial = null;

            base.Close();
        }

        internal void Open(bool forceReceiveData)
        {
            SafeOpen();

            if (forceReceiveData && _isRunningOnMono)
                _receiveSerial = new Timer(ManualSerial, null, 0, 1);
        }

        public new event EventHandler<SerialDataReceivedEventArgs> DataReceived;

        protected virtual void OnDataReceived(SerialDataReceivedEventArgs e)
        {
            DataReceived?.Invoke(this, e);
        }
    }

}