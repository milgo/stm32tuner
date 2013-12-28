using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.IO;
using System.Windows.Forms.DataVisualization.Charting;

namespace SerialViewer
{
    public partial class Form1 : Form
    {
        private SerialPort serialPort;
        private string globalBuf;
        private bool connected = false;
        private delegate void DrawChartDelegate(Int16[] t);
        
        DataPoint maxDp = null;

        public Form1()
        {
            InitializeComponent();
        }

        public int SetPort(string comPort)
        {
            try
            {
                serialPort = new SerialPort(comPort, 115200, Parity.None, 8, StopBits.One);
                serialPort.DataReceived += new SerialDataReceivedEventHandler(SerialPortDataReceived);
                serialPort.Open();
                return 0;
            }
            catch(IOException e)
            {
                MessageBox.Show(e.Message, "Connection Error",
                    MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return 1;
            }
        }

        private void DrawChart(Int16[] t)
        {
            double[] maxY = new double[1];
            maxY[0] = int.MinValue;
            Int32 maxX = 0;
            DataPoint maxDp = null;
            for (int i = 0; i < t.Length; i++)
            {
                DataPoint dp = new DataPoint();
                dp.XValue = i;
                double[] dt = new double[1];
                dt[0] = t[i];
                dp.YValues = dt;

                if (chart1.Series["Series1"].Points.Count < i + 1)
                {
                    chart1.Series["Series1"].Points.Add(dp);
                }
                else
                {
                    chart1.Series["Series1"].Points[i] = dp;
                }

                if (dp.YValues.Length > 0)
                {
                    if (dp.YValues[0] >= maxY[0])
                    {
                        maxX = i;
                        maxY[0] = dp.YValues[0];
                        maxDp = dp;
                    }
                }
            }

            maxDp.IsValueShownAsLabel = true;
            maxDp.Label = maxX.ToString();
        }

        private void SerialPortDataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            globalBuf += serialPort.ReadExisting();
            
            while (globalBuf.Length >= 3072 + 2)
            {
                string[] tokens0 = globalBuf.Split('$');
                
                
                if (tokens0.Length > 0)
                {

                    for (int i = 0; i < tokens0.Length; i++)
                    {
                        if (tokens0[i].Length == 3072)
                        {
                            string[] tokens1 = tokens0[i].Split('x');

                            Int16[] t = new Int16[tokens1.Length];
                            for (int j = 0; j < tokens1.Length; j++)
                            {
                                if (tokens1[j].Length > 0)
                                {
                                    t[j] = System.Int16.Parse(tokens1[j], System.Globalization.NumberStyles.AllowHexSpecifier);
                                }                                
                            }

                            DrawChartDelegate dcd = new DrawChartDelegate(DrawChart);
                            this.Invoke(dcd, new object[] { t });
                        }
                    }
                }

                if (tokens0[tokens0.Length - 1].Length < 3072)
                {
                    globalBuf = tokens0[tokens0.Length - 1];
                }
                else globalBuf = "";
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (!connected)
            {
                String com = (String)comboBox1.SelectedItem;
                if (SetPort(com) == 0)
                {
                    connected = true;
                    button1.Text = "Disconnect";
                }
            }
            else
            {
                button1.Text = "Connect";
                serialPort.Close();
                connected = false;
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            DataPoint dp = new DataPoint();
            dp.XValue = 4;
            double[] dt = new double[1];
            dt[0] = 40;
            dp.YValues = dt;
            chart1.Series["Series1"].Points.Add(dp);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            for (int i = 0; i < 1024; i++)
            {
                DataPoint dp = new DataPoint();
                dp.XValue = i;
                double[] dt = new double[1];
                dt[0] = 0;
                dp.YValues = dt;
                chart1.Series["Series1"].Points.Add(dp);
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (connected)
            {
                serialPort.Close();
                connected = false;
            }
        }


    }
}
