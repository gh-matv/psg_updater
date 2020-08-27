using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using WpfApp1.Antivirus;

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

            AntivirusAdvisor();

            bool createdNew;
            Globals.mutex = new Mutex(true, "psg_launcher", out createdNew);
            if (!createdNew)
            {
                WindowHelper.BringAllProcessToFront(Process.GetCurrentProcess().ProcessName);
                this.Close();
                return;
            }

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
        public static extern bool startupdate([MarshalAs(UnmanagedType.LPStr)] StringBuilder stringBuilder);

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
        enum errMessage : int
        {
            ERRM_UNAUTHORIZED = 1,
            ERRM_NOSERVERAVAILABLE,
            ERRM_USERISBANNED,
            ERRM_NOCREATESERVFILE
        };

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
            Process.Start("https://streetgears.eu/Auth/forgotpassword");
        }


        public void Btn_logout_MouseUp(object sender, MouseButtonEventArgs e)
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

        public static void AntivirusAdvisor()
        {
            if (!Properties.Settings.Default.antivirus_message_shown && !Antivirus.WinDefender.CheckForExclusion(AppDomain.CurrentDomain.BaseDirectory))
            {
                switch (MessageBox.Show("You must add an exception to your antivirus to play the game.\nDo you want to do it automatically ?", "PSG - Warning [0441]", MessageBoxButton.YesNo))
                {
                    case MessageBoxResult.Yes:
                        Antivirus.WinDefender.AddExclusion(AppDomain.CurrentDomain.BaseDirectory);
                        break;
                    case MessageBoxResult.No:
                        MessageBox.Show($"Please add the following path as an exclusion in your antivirus software :\n{AppDomain.CurrentDomain.BaseDirectory}", "PSG - Info [0442]");
                        break;
                }
                Properties.Settings.Default.antivirus_message_shown = true;
                Properties.Settings.Default.Save();
            }
        }

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

        private string selectedServer = "";
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

            if(!s.ToString().Contains(","))
            {
                // No server is sent back by the server. That means either none is available by the player
                //  or the token is invalid. We might add a check serverside and here later.
                // TODO

                //MessageBox.Show("Invalid answer from dll !");
                switch(int.Parse(s.ToString()))
                {
                    case (int)errMessage.ERRM_UNAUTHORIZED:
                        // MessageBox.Show("Error while trying to connect to the server. Please login again.");
                        Btn_logout_MouseUp(null, null);
                        break;
                    case (int)errMessage.ERRM_NOSERVERAVAILABLE:
                        MessageBox.Show("The server is currently in maintenance mode. Please try again later and check the discord for more details.", "PSG - Maintenance [7284]");
                        break;
                    case (int)errMessage.ERRM_USERISBANNED:
                        MessageBox.Show("You are banned from this game.", "PSG - Info [0019]");
                        break;
                    case (int)errMessage.ERRM_NOCREATESERVFILE:
                        MessageBox.Show("Unable to save authorization file. Please start again in admin mode, and login again.", "PSG - Error [1179]");
                        Btn_logout_MouseUp(null, null);
                        break;
                    default:
                        MessageBox.Show("An unknown error happened.", "PSG - Error [0001-"+s.ToString()+"]");
                        break;
                }
                Environment.Exit(0);
                // Btn_logout_MouseUp(null, null);
                return;
            }

            var servers = s.ToString().Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
            if(servers.Length > 1)
            {
                // Display the server list on the UI.
                list_servers.Visibility = Visibility.Visible;
                lbl_serverselection.Visibility = Visibility.Visible;
                list_servers.SelectionChanged += OnSelectServer;
                foreach(var server in servers)
                {
                    list_servers.Items.Add(server);
                }
                return;
            }

            selectedServer = servers[0];
            AfterServerChoiceCheck();
        }

        private void OnSelectServer(object sender, SelectionChangedEventArgs e)
        {
            if (list_servers.SelectedItem == null) return;
            selectedServer = (string)list_servers.SelectedItem;
            list_servers.Visibility = Visibility.Hidden;
            lbl_serverselection.Visibility = Visibility.Hidden;
            AfterServerChoiceCheck();
        }

        private void AfterServerChoiceCheck()
        {
            if(selectedServer == "")
            {
                MessageBox.Show("Wrong selected server !");
                return;
            }

            SetProgressBar(true);

            if (!quickuptodatecheck())
            {
                var x = new StringBuilder();
                x.Append(selectedServer);
                if (!startupdate(x))
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

            // Try to automatically deduce the language, based on computer's UI language.
            // Else, open the "options" menu to let the user select his language.
            if(Properties.Settings.Default.lang < 900)
            {
                switch(CultureInfo.InstalledUICulture.IetfLanguageTag.Split('-')[0])
                {
                    case "fr":
                        Properties.Settings.Default.lang = 1147;
                        break;

                    case "en":
                        Properties.Settings.Default.lang = 949;
                        break;

                    case "de":
                        Properties.Settings.Default.lang = 1141;
                        break;

                    default:
                        var p = new Params();
                        p.ShowDialog();
                        return;
                }
                Properties.Settings.Default.Save();
            }

            startgame(Properties.Settings.Default.lang);
            Environment.Exit(0);
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

        private void Sai_login_TextChanged(object sender, TextChangedEventArgs e)
        {

        }

        private void sai_pass_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.Key == Key.Enter)
            {
                Btn_login_MouseUp(null, null);
            }
        }

    }
}
