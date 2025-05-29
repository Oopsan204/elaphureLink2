using System;
using Microsoft.Win32;
using System.IO;
using System.Reflection;
using System.Diagnostics;

namespace elaphureLink.Wpf.Core.Services
{
    public interface IStartupService
    {
        bool IsStartupEnabled { get; }
        bool SetStartupEnabled(bool enabled);
    }

    public class StartupService : IStartupService
    {
        private const string APP_NAME = "elaphureLink";
        private readonly string _appPath;

        public StartupService()
        {
            _appPath = Process.GetCurrentProcess().MainModule.FileName;
        }

        public bool IsStartupEnabled
        {
            get
            {
                try
                {
                    using (RegistryKey key = Registry.CurrentUser.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", true))
                    {
                        return key?.GetValue(APP_NAME) != null;
                    }
                }
                catch
                {
                    return false;
                }
            }
        }

        public bool SetStartupEnabled(bool enabled)
        {
            try
            {
                using (RegistryKey key = Registry.CurrentUser.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", true))
                {
                    if (enabled)
                    {
                        key?.SetValue(APP_NAME, $"\"{_appPath}\" --minimized");
                    }
                    else
                    {
                        key?.DeleteValue(APP_NAME, false);
                    }
                    return true;
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to set startup: {ex.Message}");
                return false;
            }
        }
    }
}
