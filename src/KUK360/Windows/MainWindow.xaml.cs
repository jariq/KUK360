/* MIT License
 * 
 * KUK360 - Simple 360 photo viewer for Windows
 * Copyright (c) 2019-2024 Jaroslav Imrich <jimrich@jimrich.sk>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

using KUK360.Codes;
using Microsoft.VisualBasic.FileIO;
using Microsoft.Win32;
using System;
using System.Collections.Specialized;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace KUK360.Windows
{
    public partial class MainWindow : Window
    {
        internal static Window LastInstance
        {
            get;
            private set;
        }

        private FullScreenManager _fullScreenManager = null;

        private FileManager _fileManager = new FileManager();

        private ProjectionManager _projectionManager = new ProjectionManager();

        private string _draggedFile = null;

        public MainWindow()
        {
            LastInstance = this;

            InitializeComponent();

            Application.Current.DispatcherUnhandledException += GlobalExceptionHandler;

            _fullScreenManager = new FullScreenManager(this);

            // Set data contexts
            menuItemAutomaticDetection.DataContext = _projectionManager;
            ((ContextMenu)this.Resources["ViewerContexMenu"]).DataContext = _projectionManager;

            // Attach to this event so bubble can be show when "AutoDetectionEnabled" property is changed via context menu and application menu
            _projectionManager.PropertyChanged += ProjectionManager_PropertyChanged;

            // Set focus or else context menu won't be receiving events from commands
            FocusManager.SetFocusedElement(this, this);

            ViewFile(null);

            if (Environment.GetCommandLineArgs().Length > 1)
            {
                string argPath = Environment.GetCommandLineArgs()[1];
                if (!string.IsNullOrEmpty(argPath) && File.Exists(argPath))
                {
                    if (_fileManager.IsExtSupported(Path.GetExtension(argPath)))
                    {
                        ViewFile(argPath);
                    }
                }
            }
        }

        private void GlobalExceptionHandler(object sender, System.Windows.Threading.DispatcherUnhandledExceptionEventArgs e)
        {
            e.Handled = true;
            HandleException(e.Exception);
        }

        private void HandleException(Exception ex)
        {
            string message = string.Empty;
            message += "Application encountered unexpected error." + Environment.NewLine + Environment.NewLine;
            message += "Error description: " + (ex.Message ?? "Description is not available") + Environment.NewLine + Environment.NewLine;
            message += "Application will now close.";

            MessageBoxResult result = MessageBox.Show(this, message, "KUK360", MessageBoxButton.OKCancel, MessageBoxImage.Error);
            if (result == MessageBoxResult.OK)
            {
                Application.Current.Shutdown();
            }
        }

        private void DoEvents()
        {
            // Note: This should process all windows messages currently in the message queue
            Application.Current.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Background, new Action(delegate { }));
        }

        private void ViewFile(string path)
        {
            if (string.IsNullOrEmpty(path))
            {
                _fileManager.SetCurrentFile(path);

                sphereViewer.LoadImage(null);
                sphereViewer.HideZoomControl();
                HideSphereViewer();

                flatViewer.LoadImage(null, true);
                flatViewer.HideZoomControl();
                HideFlatViewer();

                DisplayOpenButtonInTextViewer();

                this.Title = $"KUK360";
            }
            else
            {
                try
                {
                    bubbleViewer.ShowPermanentBubble("Loading...");
                    DoEvents();

                    _fileManager.SetCurrentFile(path);

                    try
                    {
                        ProjectionType projection = _projectionManager.DetermineImageProjection(path);
                        if (projection == ProjectionType.Sphere)
                        {

                            sphereViewer.LoadImage(path);
                            DisplayImageInSphereViewer();
                            flatViewer.LoadImage(path, true);
                        }
                        else
                        {
                            flatViewer.LoadImage(path, true);
                            DisplayImageInFlatViewer();
                            sphereViewer.LoadImage(path);
                        }
                    }
                    catch (ImageLoadingException)
                    {
                        sphereViewer.LoadImage(null);
                        sphereViewer.HideZoomControl();
                        HideSphereViewer();

                        flatViewer.LoadImage(null, true);
                        flatViewer.HideZoomControl();
                        HideFlatViewer();

                        bubbleViewer.HideBubble();
                        DisplayStaticTextInTextViewer("Unable to load image");
                    }

                    this.Title = $"KUK360 - {path}";
                }
                catch
                {
                    // Reset window to initial view and rethrow exception
                    ViewFile(null);
                    throw;
                }
            }
        }

        private bool IsImageDisplayed()
        {
            return (!string.IsNullOrEmpty(_fileManager.CurrentFile) && (IsSphereViewerDisplayed() || IsFlatViewerDisplayed()));
        }

        #region SphereViewer

        private bool IsSphereViewerDisplayed()
        {
            return (sphereViewer.Visibility == Visibility.Visible);
        }

        private void DisplayImageInSphereViewer()
        {
            KUK360.ExternalCodes.FontAwesome.WPF.Awesome.SetContent(btnProjectionSwitch, KUK360.ExternalCodes.FontAwesome.WPF.FontAwesomeIcon.Image);

            HideTextViewer();
            HideFlatViewer();
            ShowSphereViewer();

            bubbleViewer.ShowTemporaryBubble("360° projection");
        }

        private void ShowSphereViewer()
        {
            sphereViewer.Visibility = Visibility.Visible;
        }

        private void HideSphereViewer()
        {
            sphereViewer.Visibility = Visibility.Collapsed;
        }

        #endregion

        #region FlatViewer

        private bool IsFlatViewerDisplayed()
        {
            return (flatViewer.Visibility == Visibility.Visible);
        }

        private void DisplayImageInFlatViewer()
        {
            KUK360.ExternalCodes.FontAwesome.WPF.Awesome.SetContent(btnProjectionSwitch, KUK360.ExternalCodes.FontAwesome.WPF.FontAwesomeIcon.Globe);

            HideTextViewer();
            ShowFlatViewer();
            HideSphereViewer();

            bubbleViewer.ShowTemporaryBubble("Flat projection");
        }

        private void ShowFlatViewer()
        {
            flatViewer.Visibility = Visibility.Visible;
        }

        private void HideFlatViewer()
        {
            flatViewer.Visibility = Visibility.Collapsed;
        }

        #endregion

        #region TextViewer

        private void DisplayOpenButtonInTextViewer()
        {
            ShowTextViewer();
            textViewer.ShowOpenButton();
        }

        private void DisplayStaticTextInTextViewer(string text)
        {
            ShowTextViewer();
            textViewer.ShowStaticText(text);
        }

        private void ShowTextViewer()
        {
            textViewer.Visibility = Visibility.Visible;
        }

        private void HideTextViewer()
        {
            textViewer.Visibility = Visibility.Collapsed;
        }

        #endregion

        #region Drag and drop

        private void GridMain_Drop(object sender, DragEventArgs e)
        {
            if (_draggedFile != null)
            {
                try
                {
                    ViewFile(_draggedFile);
                }
                catch (Exception ex)
                {
                    // Note: Drag-and-drop in WPF is handled through the standard unmanaged OLE drag-drop mechanism 
                    //       and because of that GlobalExceptionHandler won't catch exceptions generated in this event handler.
                    HandleException(ex);
                }
            }
        }

        private void GridMain_DragEnter(object sender, DragEventArgs e)
        {
            _draggedFile = null;

            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
                if (files != null && files.Length == 1)
                {
                    string file = files[0];
                    if (File.Exists(file) && _fileManager.IsExtSupported(Path.GetExtension(file)))
                    {
                        _draggedFile = file;
                    }
                }
            }

            e.Handled = true;
        }

        private void GridMain_DragOver(object sender, DragEventArgs e)
        {
            e.Effects = (_draggedFile == null) ? DragDropEffects.None : DragDropEffects.Copy;
            e.Handled = true;
        }

        #endregion

        #region CmdFileOpen

        public static RoutedCommand CmdFileOpen = new RoutedCommand();

        private void CmdFileOpenCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdFileOpenExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Title = "Select image to be displayed";
            openFileDialog.Filter = _fileManager.ImageFormatManager.OpenFileDialogFilter;
            openFileDialog.FilterIndex = _fileManager.ImageFormatManager.SupportedExtensions.Count + 1;

            if (IsImageDisplayed())
                openFileDialog.InitialDirectory = Path.GetDirectoryName(_fileManager.CurrentFile);

            if (openFileDialog.ShowDialog() == true)
            {
                string selectedFile = openFileDialog.FileName;
                ViewFile(selectedFile);
            }
        }

        #endregion

        #region CmdFilePrevious

        public static RoutedCommand CmdFilePrevious = new RoutedCommand();

        private void CmdFilePreviousCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !string.IsNullOrEmpty(_fileManager.PreviousFile);
        }

        private void CmdFilePreviousExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            ViewFile(_fileManager.PreviousFile);
        }

        #endregion

        #region CmdFileNext

        public static RoutedCommand CmdFileNext = new RoutedCommand();

        private void CmdFileNextCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !string.IsNullOrEmpty(_fileManager.NextFile);
        }

        private void CmdFileNextExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            ViewFile(_fileManager.NextFile);
        }

        #endregion

        #region CmdFileClose

        public static RoutedCommand CmdFileClose = new RoutedCommand();

        private void CmdFileCloseCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !string.IsNullOrEmpty(_fileManager.NextFile);
        }

        private void CmdFileCloseExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            ViewFile(null);
        }

        #endregion

        #region CmdFileOpenLocation

        public static RoutedCommand CmdFileOpenLocation = new RoutedCommand();

        private void CmdFileOpenLocationCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdFileOpenLocationExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            Process.Start("explorer.exe", $"/select,\"{_fileManager.CurrentFile}\"");
        }

        #endregion

        #region CmdFileExit

        public static RoutedCommand CmdFileExit = new RoutedCommand();

        private void CmdFileExitCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdFileExitExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        #endregion

        #region CmdEditCopy

        public static RoutedCommand CmdEditCopy = new RoutedCommand();

        private void CmdEditCopyCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdEditCopyExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            StringCollection paths = new StringCollection();
            paths.Add(_fileManager.CurrentFile);

            Clipboard.Clear();
            Clipboard.SetFileDropList(paths);
        }

        #endregion

        #region CmdEditDelete

        public static RoutedCommand CmdEditDelete = new RoutedCommand();

        private void CmdEditDeleteCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdEditDeleteExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            MessageBoxResult result = MessageBox.Show(this, $"Do you really want to move displayed image {Path.GetFileName(_fileManager.CurrentFile)} to the recycle bin?", "KUK360", MessageBoxButton.YesNo, MessageBoxImage.Question);
            if (result == MessageBoxResult.Yes)
            {
                FileSystem.DeleteFile(_fileManager.CurrentFile, UIOption.OnlyErrorDialogs, RecycleOption.SendToRecycleBin);
                ViewFile(_fileManager.NextFile);
            }
        }

        #endregion

        #region CmdProjectionSwitch

        public static RoutedCommand CmdProjectionSwitch = new RoutedCommand();

        private void CmdProjectionSwitchCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdProjectionSwitchExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsSphereViewerDisplayed())
            {
                _projectionManager.Projection = ProjectionType.Flat;
                DisplayImageInFlatViewer();

                sphereViewer.AutoRotateStop();
            }
            else
            {
                _projectionManager.Projection = ProjectionType.Sphere;
                DisplayImageInSphereViewer();
            }
        }

        #endregion

        #region CmdProjectionAutomatic

        public static RoutedCommand CmdProjectionAutomatic = new RoutedCommand();

        private void CmdProjectionAutomaticCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdProjectionAutomaticExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            _projectionManager.AutoDetectionEnabled = !_projectionManager.AutoDetectionEnabled;

            bubbleViewer.ShowTemporaryBubble((_projectionManager.AutoDetectionEnabled ? "Enabled" : "Disabled") + " projection detection");
        }

        private void ProjectionManager_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName == "AutoDetectionEnabled")
            {
                bubbleViewer.ShowTemporaryBubble((_projectionManager.AutoDetectionEnabled ? "Enabled" : "Disabled") + " projection detection");
            }
        }

        #endregion

        #region CmdZoomIn

        public static RoutedCommand CmdZoomIn = new RoutedCommand();

        private void CmdZoomInCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdZoomInExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsFlatViewerDisplayed())
            {
                flatViewer.ZoomIn();
            }
            else
            {
                sphereViewer.ZoomIn();
            }
        }

        #endregion

        #region CmdZoomOut

        public static RoutedCommand CmdZoomOut = new RoutedCommand();

        private void CmdZoomOutCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdZoomOutExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsFlatViewerDisplayed())
            {
                flatViewer.ZoomOut();
            }
            else
            {
                sphereViewer.ZoomOut();
            }
        }

        #endregion

        #region CmdZoom100

        public static RoutedCommand CmdZoom100 = new RoutedCommand();

        private void CmdZoom100CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdZoom100Executed(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsFlatViewerDisplayed())
            {
                flatViewer.ZoomReset();
            }
            else
            {
                sphereViewer.ZoomReset();
            }
        }

        #endregion

        #region CmdZoomShow

        public static RoutedCommand CmdZoomShow = new RoutedCommand();

        private void CmdZoomShowCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdZoomShowExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            sphereViewer.ToggleZoomControl();
            flatViewer.ToggleZoomControl();
        }

        #endregion

        #region CmdRotateLeft

        public static RoutedCommand CmdRotateLeft = new RoutedCommand();

        private void CmdRotateLeftCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            if (IsFlatViewerDisplayed())
            {
                e.CanExecute = IsImageDisplayed();
            }

            if (IsSphereViewerDisplayed())
            {
                e.CanExecute = IsImageDisplayed() && sphereViewer.CanAutoRotateLeft();
            }
        }

        private void CmdRotateLeftExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsFlatViewerDisplayed())
            {
                flatViewer.RotateLeft();
            }

            if (IsSphereViewerDisplayed())
            {
                sphereViewer.AutoRotateLeft();
                bubbleViewer.ShowTemporaryBubble("Rotation speed: " + Math.Abs(sphereViewer.GetAutoRotateSpeed()));
            }
        }

        #endregion

        #region CmdRotateRight

        public static RoutedCommand CmdRotateRight = new RoutedCommand();

        private void CmdRotateRightCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            if (IsFlatViewerDisplayed())
            {
                e.CanExecute = IsImageDisplayed();
            }

            if (IsSphereViewerDisplayed())
            {
                e.CanExecute = IsImageDisplayed() && sphereViewer.CanAutoRotateRight();
            }
        }

        private void CmdRotateRightExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsFlatViewerDisplayed())
            {
                flatViewer.RotateRight();
            }

            if (IsSphereViewerDisplayed())
            {
                sphereViewer.AutoRotateRight();
                bubbleViewer.ShowTemporaryBubble("Rotation speed: " + Math.Abs(sphereViewer.GetAutoRotateSpeed()));
            }
        }

        #endregion

        #region CmdViewReset

        public static RoutedCommand CmdViewReset = new RoutedCommand();

        private void CmdViewResetCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdViewResetExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsSphereViewerDisplayed())
            {
                sphereViewer.LoadImage(_fileManager.CurrentFile);
            }
            else
            {
                flatViewer.LoadImage(_fileManager.CurrentFile, false);
            }
        }

        #endregion

        #region CmdViewFullScreen

        public static RoutedCommand CmdViewFullScreen = new RoutedCommand();

        private void CmdViewFullScreenCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdViewFullScreenExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            _fullScreenManager.ToggleFullScreenMode();
        }

        #endregion

        #region CmdExitFullScreen

        public static RoutedCommand CmdExitFullScreen = new RoutedCommand();

        private void CmdExitFullScreenCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed() && _fullScreenManager.IsInFullScreenMode;
        }

        private void CmdExitFullScreenExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            _fullScreenManager.ExitFullScreenMode();
        }

        #endregion

        #region CmdHelpVisitWebsite

        public static RoutedCommand CmdHelpVisitWebsite = new RoutedCommand();

        private void CmdHelpVisitWebsiteCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdHelpVisitWebsiteExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            Process.Start(new ProcessStartInfo(@"https://www.kuk360.com/"));
        }

        #endregion

        #region CmdHelpAbout

        public static RoutedCommand CmdHelpAbout = new RoutedCommand();

        private void CmdHelpAboutCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdHelpAboutExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            AboutWindow aboutWindow = new AboutWindow();
            aboutWindow.Owner = this;
            aboutWindow.ShowDialog();
        }

        #endregion
    }
}
