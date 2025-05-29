using System;
using System.Drawing;
using System.Windows;
using System.Windows.Forms;
using elaphureLink.Wpf.Core.Services;

namespace elaphureLink.Wpf.Core.Services
{    public interface ISystemTrayService
    {
        event Action ShowMainWindow;
        event Action ExitApplication;
        
        void Initialize();
        void ShowNotification(string title, string message, ToolTipIcon icon = ToolTipIcon.Info);
        void SetIcon(Icon icon);
        void Dispose();
    }    public class SystemTrayService : ISystemTrayService, IDisposable
    {
        private NotifyIcon _notifyIcon;
        private readonly IStartupService _startupService;
        
        public event Action ShowMainWindow;
        public event Action ExitApplication;

        public SystemTrayService(IStartupService startupService)
        {
            _startupService = startupService;
        }

        public void Initialize()
        {
            _notifyIcon = new NotifyIcon
            {
                Text = "elaphureLink",
                Visible = true,
                Icon = SystemIcons.Application // Default icon
            };            // Create context menu
            var contextMenu = new ContextMenuStrip();
            
            // Show/Hide window
            contextMenu.Items.Add("Show/Hide Window", null, (s, e) => ShowMainWindow?.Invoke());
            contextMenu.Items.Add("-"); // Separator
            
            // Auto startup
            var startupItem = new ToolStripMenuItem("Start with Windows")
            {
                Checked = _startupService.IsStartupEnabled(),
                CheckOnClick = true
            };
            startupItem.Click += (s, e) => {
                try 
                {
                    if (startupItem.Checked)
                        _startupService.EnableStartup();
                    else
                        _startupService.DisableStartup();
                }
                catch (Exception ex)
                {
                    ShowNotification("Startup Error", $"Failed to change startup setting: {ex.Message}", ToolTipIcon.Error);
                    startupItem.Checked = !startupItem.Checked; // Revert change
                }
            };
            contextMenu.Items.Add(startupItem);
            
            contextMenu.Items.Add("-"); // Separator
            
            // Connection status (placeholder)
            var statusItem = new ToolStripMenuItem("Status: Disconnected")
            {
                Enabled = false
            };
            contextMenu.Items.Add(statusItem);
            
            contextMenu.Items.Add("-"); // Separator
            contextMenu.Items.Add("Exit", null, (s, e) => ExitApplication?.Invoke());

            _notifyIcon.ContextMenuStrip = contextMenu;
            _notifyIcon.DoubleClick += (s, e) => ShowMainWindow?.Invoke();        }

        public void ShowNotification(string title, string message, ToolTipIcon icon = ToolTipIcon.Info)
        {
            _notifyIcon?.ShowBalloonTip(3000, title, message, icon);
        }

        public void SetIcon(Icon icon)
        {
            if (_notifyIcon != null)
            {
                _notifyIcon.Icon = icon;
            }
        }

        public void Dispose()
        {
            _notifyIcon?.Dispose();
        }
    }
}
