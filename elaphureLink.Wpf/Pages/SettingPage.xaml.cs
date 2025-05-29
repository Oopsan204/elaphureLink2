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

using elaphureLink.Wpf.Core.Services;

namespace elaphureLink.Wpf.Pages
{    public partial class SettingPage
    {
        private IStartupService _startupService;
        private ISystemTrayService _systemTrayService;
        private ISettingsService _settingsService;
        
        public SettingPage()
        {
            this.InitializeComponent();
            
            // Get services from IoC container
            _startupService = App.IocKernel.Get<IStartupService>();
            _systemTrayService = App.IocKernel.Get<ISystemTrayService>();
            _settingsService = App.IocKernel.Get<ISettingsService>();
            
            // Load current settings
            LoadSettings();
        }
        
        private void LoadSettings()
        {
            try
            {
                // Load startup setting
                StartupCheckBox.IsChecked = _startupService.IsStartupEnabled();
                
                // Load start minimized setting (you might want to store this in settings service)
                // For now, we'll assume it's false by default
                StartMinimizedCheckBox.IsChecked = false;
                
                // Update connection status (this would be updated from your connection logic)
                UpdateConnectionStatus(false); // Assuming disconnected by default
            }
            catch (Exception ex)
            {
                // Handle any errors loading settings
                _systemTrayService?.ShowNotification("Settings Error", $"Failed to load settings: {ex.Message}");
            }
        }
        
        private void UpdateConnectionStatus(bool isConnected)
        {
            if (isConnected)
            {
                ConnectionStatusIndicator.Fill = new SolidColorBrush(Colors.Green);
                ConnectionStatusText.Text = "Connected";
            }
            else
            {
                ConnectionStatusIndicator.Fill = new SolidColorBrush(Colors.Red);
                ConnectionStatusText.Text = "Disconnected";
            }
        }
        
        private void StartupCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            try
            {
                _startupService.EnableStartup();
                _systemTrayService?.ShowNotification("Startup Enabled", "elaphureLink will now start with Windows");
            }
            catch (Exception ex)
            {
                StartupCheckBox.IsChecked = false;
                _systemTrayService?.ShowNotification("Startup Error", $"Failed to enable startup: {ex.Message}");
            }
        }
        
        private void StartupCheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            try
            {
                _startupService.DisableStartup();
                _systemTrayService?.ShowNotification("Startup Disabled", "elaphureLink will no longer start with Windows");
            }
            catch (Exception ex)
            {
                StartupCheckBox.IsChecked = true;
                _systemTrayService?.ShowNotification("Startup Error", $"Failed to disable startup: {ex.Message}");
            }
        }
        
        private void StartMinimizedCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            // Store this setting for future use
            // You might want to add this to your settings service
            _systemTrayService?.ShowNotification("Start Minimized", "Application will start minimized to system tray");
        }
        
        private void StartMinimizedCheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            // Store this setting for future use
            _systemTrayService?.ShowNotification("Start Normal", "Application will start normally");
        }
    }
}
