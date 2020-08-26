using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Windows;
using System.Windows.Controls;

namespace WpfApp1
{
    /// <summary>
    /// Logique d'interaction pour Params.xaml
    /// </summary>
    public partial class Params : Window
    {
        public Params()
        {
            InitializeComponent();
        }

        private void UpdateSelectedLanguage()
        {

            img_gersel.Opacity = 0;
            img_ensel.Opacity = 0;
            img_frsel.Opacity = 0;

            switch (Properties.Settings.Default.lang)
            {
                case 1147:
                    img_frsel.Opacity = 1;
                    break;
                case 1141:
                    img_gersel.Opacity = 1;
                    break;
                case 949:
                    img_ensel.Opacity = 1;
                    break;
            }
        }

        private void Window_Initialized(object sender, System.EventArgs e)
        {
            UpdateSelectedLanguage();
        }

        private void Btn_verify_files_MouseUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            try
            {
                File.Delete("mnfst.sg");

                //Globals.mutex.ReleaseMutex();
                //Globals.mutex.Dispose();
                Globals.mutex.Close();

                Process.Start(AppDomain.CurrentDomain.FriendlyName);
                Environment.Exit(0);
            }
            catch (Exception)
            {
                MessageBox.Show("Unable to recheck everything. Make sure the game is closed, and no file is currently being modified.");
                return;
            }
        }

        private void Img_gersel_MouseUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Properties.Settings.Default.lang = 1141;
            Properties.Settings.Default.Save();
            UpdateSelectedLanguage();
        }

        private void Img_ensel_MouseUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Properties.Settings.Default.lang = 949;
            Properties.Settings.Default.Save();
            UpdateSelectedLanguage();
        }

        private void Img_frsel_MouseUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Properties.Settings.Default.lang = 1147;
            Properties.Settings.Default.Save();
            UpdateSelectedLanguage();
        }

        private void Btn_join_discord_MouseUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Process.Start("https://discord.com/invite/cheZMW");
        }

        private void Btn_close_MouseUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            this.Close();
        }
    }
}
