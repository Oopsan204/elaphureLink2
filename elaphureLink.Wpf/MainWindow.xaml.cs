using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using ModernWpf.Controls;

using elaphureLink.Wpf.Pages;
using elaphureLink.Wpf.Core;
using elaphureLink.Wpf.Core.Services;
using System.Diagnostics;

namespace elaphureLink.Wpf
{    public partial class MainWindow : Window
    {
        private ISystemTrayService _systemTrayService;
        
        public MainWindow()
        {
            InitializeComponent();
            
            // Get system tray service from IoC container
            _systemTrayService = App.IocKernel.Get<ISystemTrayService>();
            
            // Set up window state change handling for system tray
            this.StateChanged += MainWindow_StateChanged;
        }
        
        private void MainWindow_StateChanged(object sender, EventArgs e)
        {
            if (WindowState == WindowState.Minimized)
            {
                // Hide from taskbar when minimized
                this.ShowInTaskbar = false;
                this.Hide();
                
                // Show notification
                _systemTrayService?.ShowNotification("elaphureLink", "Application minimized to system tray");
            }
        }

        private async void Window_ClosingAsync(
            object sender,
            System.ComponentModel.CancelEventArgs e
        )
        {
            e.Cancel = true; // prevent exit

            ExitConfirmDialog dialog = new ExitConfirmDialog();
            var result = await ContentDialogMaker.CreateContentDialogAsync(dialog, false);

            if (result == ContentDialogResult.Primary)
            {
                // TODO: clean up

                // exit.
                Properties.Settings.Default.Save();
                Process.GetCurrentProcess().Kill();
            }
        }        /// <summary>Brings main window to foreground.</summary>
        public void BringToForeground()
        {
            if (this.WindowState == WindowState.Minimized || this.Visibility == Visibility.Hidden)
            {
                this.Show();
                this.WindowState = WindowState.Normal;
                this.ShowInTaskbar = true;
            }

            // According to some sources these steps guarantee that an app will be brought to foreground.
            this.Activate();
            this.Topmost = true;
            this.Topmost = false;
            this.Focus();
        }
    }
}
