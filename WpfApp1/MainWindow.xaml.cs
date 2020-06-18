using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace WpfApp1
{

    /// <summary>
    /// Logique d'interaction pour MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public const string dll = @"lnc.dll";

        public MainWindow()
        {
            SelfUpdate su = new SelfUpdate();
            su.ShowDialog();

            InitializeComponent();
            SetProgressBar(false);
            list_servers.Visibility = Visibility.Hidden;
            lbl_serverselection.Visibility = Visibility.Hidden;
            btn_start.Visibility = Visibility.Hidden;

            if (!isconnected())
            {
                SetLoginboxVisible(true);
                return;
            }

            try
            {
                openconsole();
            }
            catch(Exception e)
            {
                MessageBox.Show("An error happened !");
                MessageBox.Show(e.Message);
            }

            AfterLoginChecks();
        }

        // ========================= IMPORTS FROM DLL ========================
        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool isconnected();

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool trylogin(string login, string pass);

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool startupdate(int serverid);

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void getprogressmsg([MarshalAs(UnmanagedType.LPStr)] StringBuilder stringBuilder, int size);

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool getservers([MarshalAs(UnmanagedType.LPStr)] StringBuilder stringBuilder, int size);

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool updatedone();

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern int updateresult();

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool quickuptodatecheck();

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void logout();

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void openconsole();

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool startgame(int cp=1141);

        [DllImport(dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern float getprogresspercent();

        // ========================= EVENTS ================================

        private void Btn_close_MouseUp(object sender, MouseButtonEventArgs e)
        {
            Close();
        }

        private void Btn_login_MouseUp(object sender, MouseButtonEventArgs e)
        {
            // Call try_login

            if(sai_login.Text == "" || sai_pass.Password == "")
            {
                MessageBox.Show("Please enter your login and password.");
                return;
            }

            if(!trylogin(sai_login.Text, sai_pass.Password))
            {
                MessageBox.Show("Wrong login or password. Please try again.");
                return;
            }

            AfterLoginChecks();

        }

        private void Btn_forgotpw_MouseUp(object sender, MouseButtonEventArgs e)
        {
            Process.Start("https://streetgears.eu?c=Auth&a=forgotpassword");
        }


        private void Btn_logout_MouseUp(object sender, MouseButtonEventArgs e)
        {
            logout();
            SetLoginboxVisible(true);
            SetProgressBar(false);

            list_servers.Items.Clear();
            list_servers.Visibility = Visibility.Hidden;
            lbl_serverselection.Visibility = Visibility.Hidden;
            btn_start.Visibility = Visibility.Hidden;
        }

        private void Mainwindow_Initialized(object sender, System.EventArgs e)
        {
        }

        // ======================== ACTIONS =================================

        private void SetLoginboxVisible(bool visible)
        {
            area_login.Visibility = visible ? Visibility.Visible : Visibility.Hidden;
            btn_forgotpw.Visibility = visible ? Visibility.Visible : Visibility.Hidden;
            btn_login.Visibility = visible ? Visibility.Visible : Visibility.Hidden;
            sai_login.Visibility = visible ? Visibility.Visible : Visibility.Hidden;
            sai_pass.Visibility = visible ? Visibility.Visible : Visibility.Hidden;
            lbl_progresstext.Visibility = visible ? Visibility.Visible : Visibility.Hidden;

            btn_forgotpw.IsEnabled = visible;
            btn_forgotpw.IsEnabled = visible;
            sai_login.IsEnabled = visible;
            sai_pass.IsEnabled = visible;

            // Intentionally reversed
            btn_logout.Opacity = visible ? 0 : 1;
            btn_logout.IsEnabled = !visible;
        }

        private void SetProgressBar(bool visible, short percentage_main=0, short percentage_sub=0)
        {
            pb_main.Visibility = visible ? Visibility.Visible : Visibility.Hidden;
            pb_sub.Visibility = visible ? Visibility.Visible : Visibility.Hidden;
            lbl_progresstext.Visibility = visible ? Visibility.Visible : Visibility.Hidden;

            pb_main.IsEnabled = visible;
            pb_sub.IsEnabled = visible;
            lbl_progresstext.IsEnabled = visible;

            if(visible)
            {
                pb_main.Value = percentage_main;
                pb_sub.Value = percentage_sub;
            }
        }

        private int selectedServer = 0;
        private void AfterLoginChecks()
        {
            SetLoginboxVisible(false);

            // Show the server list if count > 1
            StringBuilder s = new StringBuilder(100);
            if(!getservers(s, s.Capacity))
            {
                MessageBox.Show("Unable to load servers !");
                return;
            }

            if(!s.ToString().Contains("|"))
            {
                //MessageBox.Show("Invalid answer from dll !");
                Btn_logout_MouseUp(null, null);
                return;
            }

            var servers = s.ToString().Split(new[] { '|' }, StringSplitOptions.RemoveEmptyEntries);
            if(servers.Length > 1)
            {
                // Display the server list on the UI.
                list_servers.Visibility = Visibility.Visible;
                lbl_serverselection.Visibility = Visibility.Visible;
                list_servers.SelectionChanged += OnSelectServer;
                foreach(var server in servers)
                {
                    list_servers.Items.Add("" + server.Split(',')[1] + "\t" + server.Split(',')[0]);
                }
                return;
            }

            selectedServer = int.Parse(servers[0].Split(',')[1]);
            AfterServerChoiceCheck();
        }

        private void OnSelectServer(object sender, SelectionChangedEventArgs e)
        {
            if (list_servers.SelectedItem == null) return;
            selectedServer = int.Parse(((string)list_servers.SelectedItem).Split('\t')[0]);
            list_servers.Visibility = Visibility.Hidden;
            lbl_serverselection.Visibility = Visibility.Hidden;
            AfterServerChoiceCheck();
        }

        private void AfterServerChoiceCheck()
        {
            if(selectedServer == 0)
            {
                MessageBox.Show("Wrong selected server !");
                return;
            }

            SetProgressBar(true);

            if (!quickuptodatecheck())
            {
                if (!startupdate(selectedServer))
                {
                    MessageBox.Show("Unable to start the udpate.");
                    return;
                }

                StringBuilder sb = new StringBuilder(200);
                string msg = "";

                new Thread(() =>
                {
                    while (!updatedone())
                    {
                        getprogressmsg(sb, sb.Capacity);
                        lbl_progresstext.Dispatcher.BeginInvoke((Action)(() => lbl_progresstext.Content = "" + sb.ToString()));

                        pb_main.Dispatcher.BeginInvoke((Action)(() => pb_main.Value = 100*getprogresspercent()));

                        Thread.Sleep(50);
                    }

                    getprogressmsg(sb, sb.Capacity);
                    lbl_progresstext.Dispatcher.BeginInvoke((Action)(() => lbl_progresstext.Content = "" + sb.ToString()));

                    btn_start.Dispatcher.BeginInvoke((Action)(() => {
                        btn_start.Visibility = Visibility.Visible;
                        SetProgressBar(false);
                    }));
                    

                }).Start();

            }
        }

        private void Btn_start_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if(Properties.Settings.Default.lang < 1140)
            {
                var p = new Params();
                p.ShowDialog();
                return;
            }

            startgame(Properties.Settings.Default.lang);
        }

        private void Grid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                //DragMove();
            }
        }

        private void Mainwindow_MouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {

        }

        private void Mainwindow_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.Source.GetType().Name == "MainWindow" && e.LeftButton == MouseButtonState.Pressed)
            {
                DragMove();
            }
        }

        private void Btn_settings_MouseUp(object sender, MouseButtonEventArgs e)
        {
            this.Hide();
            var p = new Params();
            p.ShowDialog();
            this.Show();
        }
    }
}
