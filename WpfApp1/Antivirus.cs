using System;
using System.Diagnostics;
using System.Linq;

namespace WpfApp1
{
    namespace Antivirus
    {
        class WinDefender
        {
            public static bool CheckForExclusion(string path)
            {
                ProcessStartInfo startInfo = new ProcessStartInfo();
                startInfo.FileName = @"powershell.exe";
                startInfo.Arguments = @"-inputformat none -NonInteractive -Command Get-MpPreference";
                startInfo.RedirectStandardOutput = true;
                startInfo.RedirectStandardError = true;
                startInfo.UseShellExecute = false;
                startInfo.CreateNoWindow = true;
                Process process = new Process();
                process.StartInfo = startInfo;
                process.Start();

                string output = process.StandardOutput.ReadToEnd();
                string errors = process.StandardError.ReadToEnd();

                var x = output.Split(new string[] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries);
                foreach (var y in x)
                {
                    var l = y.Split(':')[0].Trim();

                    if (l == "ExclusionPath")
                    {
                        var r = y.Substring(y.IndexOf(':') + 1);

                        if (!y.Contains("{"))
                            return false;

                        var exclusions = y.Split('{')[1].Split('}')[0].Split(',');
                        if (exclusions.Any(ex => ex == path.Substring(0,path.Length-1)))
                        {
                            return true;
                        }
                    }
                }

                return false;
            }
            public static void AddExclusion(string path)
            {
                ProcessStartInfo startInfo = new ProcessStartInfo();
                startInfo.FileName = @"powershell.exe";
                startInfo.Arguments = $"-Command Add-MpPreference -ExclusionPath \"{path.Substring(0,path.Length-1)}\"";
                startInfo.RedirectStandardOutput = true;
                startInfo.RedirectStandardError = true;
                startInfo.UseShellExecute = false;
                startInfo.CreateNoWindow = true;
                Process process = new Process();
                process.StartInfo = startInfo;
                process.Start();

                string output = process.StandardOutput.ReadToEnd();
                string errors = process.StandardError.ReadToEnd();
            }
        }
    }
}
