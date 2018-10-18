﻿using System;
using System.Threading;
using OposScanner_CCO;

namespace OPOSSampleCs10
{
    class Program
    {
        private static OPOSScannerClass scanner;
        private static String[] names = 
        {
            "Bologna-USB-HID",
            "HandScanner",
            "MagellanSC",
            "QS6000",
            "QSLScanner",
            "RS232Imager",
            "RS232Scanner",
            "TableScanner",
            "USBHHScanner",
            "USBScanner"
        };

        static void Main(string[] args)
        {
            // setup the console program to exit gracefully 
            var exitEvent = new ManualResetEvent(false);
            Console.CancelKeyPress += (sender, eventArgs) =>
            {
                eventArgs.Cancel = true;
                exitEvent.Set();
            };

            // create a scanner object
            scanner = new OPOSScannerClass();

            // subscribe to the delegate 
            scanner.DataEvent += DataEvent;

            // open, claim, and enable a given scanner
            foreach (var name in names)
            {
                scanner.Open(name);
                scanner.ClaimDevice(1000);
                if (scanner.Claimed)
                {
                    Console.WriteLine("Connected to: " + name);
                    scanner.DeviceEnabled = true;
                    scanner.DataEventEnabled = true;
                    scanner.DecodeData = true;
                    break;
                }
                scanner.Close();
            }

            // wait for exit event
            Console.WriteLine("Press \'Ctrl + C\' to quit.");
            exitEvent.WaitOne();

            // unsubscribe to the delegate 
            scanner.DataEvent -= DataEvent;

            // disable operation, release control, and close the scanner
            scanner.DataEventEnabled = false;
            scanner.ReleaseDevice();
            scanner.Close();
        }

        static private void DataEvent(int value)
        {
            Console.WriteLine("Data: " + scanner.ScanDataLabel);
            scanner.DataEventEnabled = true;
        }
    }
}