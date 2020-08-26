using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Threading;
using System.Windows.Forms;

namespace WpfApp1
{
    public partial class SelfUpdate : Form
    {

        public const bool DISABLE_UPDATE = false;
        public const bool BETA_CHANNEL = false;

        public bool done_successfully = false;

        public SelfUpdate()
        {
            InitializeComponent();
            FormBorderStyle = FormBorderStyle.None;
        }

        private bool is_new_version(string client, string server)
        {
            return client != server;
        }

        private string Hash(string hasher_path, string file)
        {
            var psi = new ProcessStartInfo();
            psi.FileName = hasher_path;
            psi.Arguments = "" + '"' + file + '"';
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardError = true;
            psi.UseShellExecute = false;

            var ps = new Process();
            ps.StartInfo = psi;
            ps.Start();

            return ps.StandardOutput.ReadToEnd().Replace("\r\n","");
        }

        private void DoStuff()
        {
            string hashCalculatorPath = Path.GetTempFileName() + ".exe";
            try
            {
                File.WriteAllBytes(hashCalculatorPath, Properties.Resources.sg2017);
            }
            catch(Exception e)
            {
                MessageBox.Show("Unable to update : " + e.Message);
                Environment.Exit(0);
            }

            WebClient wc = new WebClient();
            var result = wc.DownloadString("https://streetgears.eu/index.php?c=Api_Patcher&a=PatcherInfo");

            var exe_version_server = result.Split(',')[0];
            var dll_version_server = result.Split(',')[1];

            var exe_version_client = Hash(hashCalculatorPath, Path.GetFullPath(AppDomain.CurrentDomain.FriendlyName));

            /*
            File.WriteAllLines("./debug_for_geek.txt", new string[]
            {
                "exe_s: " + exe_version_server,
                "dll_s: " + dll_version_server,
                "exe_c: " + exe_version_client
            });
            */

            if (!File.Exists("lnc.dll"))
            {
                // download lnc.dll
                try
                {
                    wc.DownloadFile("https://streetgears.eu/index.php?c=Api_Patcher&a=PatcherDownloadDll", "lnc.dll");
                }
                catch (Exception err)
                {
                    MessageBox.Show(err.Message);
                    return;
                }
            }
            else
            {
                // lnc.dll exists, check version.
                var dll_version_client = Hash(hashCalculatorPath, "lnc.dll");
                if(is_new_version(dll_version_client, dll_version_server))
                {
                    if (File.Exists("lnc.dll")) File.Delete("lnc.dll");
                    wc.DownloadFile("https://streetgears.eu/index.php?c=Api_Patcher&a=PatcherDownloadDll", "lnc.dll");
                }
            }

            if (is_new_version(exe_version_client, exe_version_server))
            {
                // download Launcher.exe
                try
                {
                    if (File.Exists("newlauncher.bin")) File.Delete("newlauncher.bin");
                    if (File.Exists("exec.bat")) File.Delete("exec.bat");

                    wc.DownloadFile("https://streetgears.eu/index.php?c=Api_Patcher&a=PatcherDownloadExe", "newlauncher.bin");
                    //var process_name = Process.GetCurrentProcess().ProcessName;
                    var file_name = AppDomain.CurrentDomain.FriendlyName;
                    string data_bat = "@echo off"
                                    + $"\ntaskkill /F /IM {file_name} /T"
                                    + "\ndel " + file_name
                                    //+ "\necho renaming launcher files"
                                    + "\nren newlauncher.bin " + file_name
                                    + "\nstart " + file_name
                                    //+ "\necho Done !"
                                    //+ "\npause"
                                    + "\n del exec.bat"
                                    ;
                    File.WriteAllText("exec.bat", data_bat);

                    Process.Start("exec.bat");
                    Thread.Sleep(50);
                    Environment.Exit(0);
                }
                catch (Exception err)
                {
                    MessageBox.Show(err.Message);
                    return;
                }
            }

            done_successfully = true;
            this.Invoke((MethodInvoker)delegate
            {
                // close the form on the forms thread
                this.Close();
            });
        }

        private void SelfUpdate_Load(object sender, EventArgs e)
        {
            if(DISABLE_UPDATE)
            {
                done_successfully = true;
                Close();
                return;
            }
            
            new Thread(DoStuff).Start();            
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        public const int WM_NCLBUTTONDOWN = 0xA1;
        public const int HT_CAPTION = 0x2;

        [System.Runtime.InteropServices.DllImport("user32.dll")]
        public static extern int SendMessage(IntPtr hWnd, int Msg, int wParam, int lParam);
        [System.Runtime.InteropServices.DllImport("user32.dll")]
        public static extern bool ReleaseCapture();
        private void SelfUpdate_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                ReleaseCapture();
                SendMessage(Handle, WM_NCLBUTTONDOWN, HT_CAPTION, 0);
            }
        }
    }
}
