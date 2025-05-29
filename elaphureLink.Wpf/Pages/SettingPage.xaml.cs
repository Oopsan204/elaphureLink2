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
        private ISettingsService _settingsService;
        
        public SettingPage()
        {
            this.InitializeComponent();
            
            // Get services from IoC container
            _startupService = App.IocKernel.Get<IStartupService>();
            _settingsService = App.IocKernel.Get<ISettingsService>();
            
            // Load current settings
            LoadSettings();
        }
        
        private void LoadSettings()
        {
            try
            {
                // Load startup setting
                StartupCheckBox.IsChecked = _startupService.IsStartupEnabled;
                
                // Update connection status (this would be updated from your connection logic)
                UpdateConnectionStatus(false); // Assuming disconnected by default
            }
            catch (Exception ex)
            {
                // Handle any errors loading settings
                MessageBox.Show($"Failed to load settings: {ex.Message}", "Settings Error", MessageBoxButton.OK, MessageBoxImage.Error);
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
                _startupService.SetStartupEnabled(true);
                MessageBox.Show("elaphureLink will now start with Windows", "Startup Enabled", MessageBoxButton.OK, MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                StartupCheckBox.IsChecked = false;
                MessageBox.Show($"Failed to enable startup: {ex.Message}", "Startup Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
        
        private void StartupCheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            try
            {
                _startupService.SetStartupEnabled(false);
                MessageBox.Show("elaphureLink will no longer start with Windows", "Startup Disabled", MessageBoxButton.OK, MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                StartupCheckBox.IsChecked = true;
                MessageBox.Show($"Failed to disable startup: {ex.Message}", "Startup Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
        
        private void StartMinimizedCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            // This would be implemented if needed
            MessageBox.Show("Start minimized feature noted", "Feature", MessageBoxButton.OK, MessageBoxImage.Information);
        }
        
        private void StartMinimizedCheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            // This would be implemented if needed
        }
    }
}
