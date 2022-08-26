using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Media;
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
using WPFCustomMessageBox;
using System.IO;
using Newtonsoft.Json;
using Microsoft.Win32;
using System.Text.RegularExpressions;
using System.Runtime.InteropServices;
using System.ComponentModel;

namespace RiseSpawnEditor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        [DllImport("Kernel32.dll")]
        private static extern void AllocConsole();
        
        private CustomSpawn? _customSpawn;
        private string _currentFile;
        private bool _hasUnsavedChanges;

        private bool _changedProperty;

        private static readonly string _version = "1.0.0";
        public static string Version => _version;

        public MainWindow()
        {
            InitializeComponent();

            _customSpawn = null;
            _currentFile = "";
            _hasUnsavedChanges = false;

            _changedProperty = false;

            Spawn.Items.Add("A");
            Spawn.Items.Add("B");
            Spawn.Items.Add("C");

            Map.ItemsSource = EnumHelper.Map;
            Map.DisplayMemberPath = "Value";
            Map.SelectedValuePath = "Key";

            foreach (var spawn in EnumHelper.Spawns)
            {
                if (spawn.Positions.Count > 0)
                {
                    var mapButton = new MenuItem
                    {
                        Header = EnumHelper.Map[spawn.Map]
                    };

                    foreach (var pos in spawn.Positions)
                    {
                        var posButton = new MenuItem
                        {
                            Header = $"Area {pos.Area} [Sub {pos.UnqId}]"
                        };
                        
                        posButton.Click += (sender, e) =>
                        {
                            HasCoordinates.IsChecked = true;

                            _changedProperty = true;
                            XCoord.Text = pos.X.ToString();
                            YCoord.Text = pos.Y.ToString();
                            ZCoord.Text = pos.Z.ToString();
                            _changedProperty = false;
                        };
                        
                        mapButton.Items.Add(posButton);
                    }

                    InsertCoordinatesButton.Items.Add(mapButton);
                }
            }
        }

        private void NewFile(object sender, ExecutedRoutedEventArgs e)
        {
            if (_hasUnsavedChanges)
            {
                var result = WarnUnsavedChanges();

                if (result == MessageBoxResult.Yes && _currentFile != "")
                {
                    SaveFile(null, null);
                }
                else if (result == MessageBoxResult.Cancel)
                {
                    return;
                }
            }

            _customSpawn = new CustomSpawn();
            PopulateEditorFields();

            SaveFileDialog dlg = new()
            {
                CheckFileExists = false,
                CheckPathExists = false,
                AddExtension = true,
                Filter = "Spawn Files | *.json",
                Title = "Save Spawn File"
            };

            if (dlg.ShowDialog() == true)
            {
                string path = System.IO.Path.GetDirectoryName(dlg.FileName)!;
                if (!Directory.Exists(path))
                {
                    Directory.CreateDirectory(path);
                }

                _currentFile = dlg.FileName;
                Title = "Spawn Editor - " + System.IO.Path.GetFileName(_currentFile);

                using StreamWriter writer = new(_currentFile, false, Encoding.UTF8);
                writer.Write(JsonConvert.SerializeObject(_customSpawn, Formatting.Indented));
                RemoveUnsavedChanges();
            }
        }

        private void OpenFile(object sender, ExecutedRoutedEventArgs e)
        {
            OpenFileDialog dlg = new()
            {
                CheckFileExists = true,
                Title = "Open Spawn File",
                Filter = "Spawn Files | *.json"
            };

            if (dlg.ShowDialog() == true)
            {
                LoadFile(dlg.FileName);
            }
        }

        private void SaveFile(object? sender, ExecutedRoutedEventArgs? e)
        {
            if (_customSpawn != null)
            {
                if (_currentFile != "")
                {
                    StoreEditorFields();

                    using StreamWriter writer = new(_currentFile, false, Encoding.UTF8);
                    writer.Write(JsonConvert.SerializeObject(_customSpawn, Formatting.Indented));
                    RemoveUnsavedChanges();
                }
                else
                {
                    SaveFileAs(sender, e);
                }
            }
            else
            {
                MessageBox.Show("Open a Spawn first", "Can't Save", MessageBoxButton.OK, MessageBoxImage.Information);
            }
        }

        private void SaveFileAs(object? sender, ExecutedRoutedEventArgs? e)
        {
            if (_customSpawn != null)
            {
                SaveFileDialog dlg = new()
                {
                    CheckFileExists = false,
                    CheckPathExists = false,
                    AddExtension = true,
                    Filter = "Quest Files | *.json",
                    Title = "Save Quest File"
                };

                if (dlg.ShowDialog() == true)
                {
                    string path = System.IO.Path.GetDirectoryName(dlg.FileName)!;
                    if (!Directory.Exists(path))
                    {
                        Directory.CreateDirectory(path);
                    }

                    _currentFile = dlg.FileName;
                    Title = "Spawn Editor - " + System.IO.Path.GetFileName(_currentFile);

                    SaveFile(sender, e);
                }
            }
            else
            {
                MessageBox.Show("Open a Quest first", "Can't Save", MessageBoxButton.OK, MessageBoxImage.Information);
            }
        }

        private void ExitProgram(object sender, ExecutedRoutedEventArgs e)
        {
            Close();
        }

        private void LoadFile(string path)
        {
            if (_hasUnsavedChanges)
            {
                var result = WarnUnsavedChanges();

                if (result == MessageBoxResult.Yes && _currentFile != "")
                {
                    SaveFile(null, null);
                }
                else if (result == MessageBoxResult.Cancel)
                {
                    return;
                }
            }

            using StreamReader sr = new(path, Encoding.UTF8);
            _customSpawn = JsonConvert.DeserializeObject<CustomSpawn>(sr.ReadToEnd());

            if (_customSpawn != null)
            {
                _currentFile = path;
                Title = "Spawn Editor - " + System.IO.Path.GetFileName(_currentFile);

                PopulateEditorFields();
                RemoveUnsavedChanges();
            }
        }

        private static MessageBoxResult WarnUnsavedChanges()
        {
            SystemSounds.Exclamation.Play();
            return CustomMessageBox.ShowYesNoCancel("You have unsaved changes. Continue anyway?", "Warning", "Save", "Don't Save", "Cancel", MessageBoxImage.Warning);
        }

        private void SetUnsavedChanges()
        {
            if (!_hasUnsavedChanges)
            {
                if (_currentFile != "")
                {
                    Title += "*";
                    _hasUnsavedChanges = true;
                }
                else
                {
                    RemoveUnsavedChanges();
                }
            }
        }

        private void RemoveUnsavedChanges()
        {
            if (_hasUnsavedChanges)
            {
                Title = Title.Replace("*", "");
                _hasUnsavedChanges = false;
            }
        }

        private void PopulateEditorFields()
        {
            if (_customSpawn != null)
            {
                QuestID.Text = _customSpawn.QuestID.ToString();
                Map.SelectedIndex = _customSpawn.Map;

                _changedProperty = true;
                Setter.Items.Clear();
                foreach (var setter in _customSpawn.Setters)
                {
                    _changedProperty = true;
                    Setter.Items.Add(setter.SetName);
                }

                if (Setter.Items.Count > 0)
                {
                    Setter.SelectedIndex = 0;

                    SetName.Text = _customSpawn.Setters[Setter.SelectedIndex].SetName;
                    Spawn.SelectedIndex = 0;

                    var spawn = _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex];

                    SpawnChance.Text = spawn.Chance.ToString();
                    SpawnArea.Text = spawn.Area.ToString();
                    SubSpawn.Text = spawn.SubSpawn.ToString();
                    
                    if (spawn.Coordinates != null)
                    {
                        var coords = spawn.Coordinates;

                        XCoord.IsEnabled = true;
                        YCoord.IsEnabled = true;
                        ZCoord.IsEnabled = true;
                        
                        XCoord.Text = coords.X.ToString();
                        YCoord.Text = coords.Y.ToString();
                        ZCoord.Text = coords.Z.ToString();
                    }
                    else
                    {
                        XCoord.IsEnabled = false;
                        YCoord.IsEnabled = false;
                        ZCoord.IsEnabled = false;
                        
                        XCoord.Text = "0";
                        YCoord.Text = "0";
                        ZCoord.Text = "0";
                    }
                }
            }

            _changedProperty = false;
        }

        private void StoreEditorFields()
        {
            if (_customSpawn != null)
            {
                _customSpawn.QuestID = int.Parse(QuestID.Text);
                _customSpawn.Map = Map.SelectedIndex;

                int setterIdx = Setter.SelectedIndex;
                int spawnIdx = Spawn.SelectedIndex;

                _customSpawn.Setters[setterIdx].SetName = SetName.Text;
                _customSpawn.Setters[setterIdx].Spawns[spawnIdx].Chance = int.Parse(SpawnChance.Text);
                _customSpawn.Setters[setterIdx].Spawns[spawnIdx].Area = int.Parse(SpawnArea.Text);
                _customSpawn.Setters[setterIdx].Spawns[spawnIdx].SubSpawn = int.Parse(SubSpawn.Text);

                if (HasCoordinates.IsEnabled)
                {

                    var spawn = _customSpawn.Setters[setterIdx].Spawns[spawnIdx];

                    if (spawn.Coordinates == null)
                    {
                        spawn.Coordinates = new IndividualSpawn.Position
                        {
                            X = double.Parse(XCoord.Text),
                            Y = double.Parse(YCoord.Text),
                            Z = double.Parse(ZCoord.Text)
                        };
                    }
                    else
                    {
                        spawn.Coordinates.X = double.Parse(XCoord.Text);
                        spawn.Coordinates.Y = double.Parse(YCoord.Text);
                        spawn.Coordinates.Z = double.Parse(ZCoord.Text);
                    }
                }
                else
                {
                    _customSpawn.Setters[setterIdx].Spawns[spawnIdx].Coordinates = null;
                }
            }
        }

        private void AddSetter_Click(object sender, RoutedEventArgs e)
        {
            if (_customSpawn != null)
            {
                SetUnsavedChanges();
                _customSpawn.Setters.Add(new SpawnSetter($"Setter{_customSpawn.Setters.Count}"));

                _changedProperty = true;
                Setter.Items.Clear();
                foreach (var setter in _customSpawn.Setters)
                {
                    _changedProperty = true;
                    Setter.Items.Add(setter.SetName);
                }

                _changedProperty = false;

                Setter.SelectedIndex = _customSpawn.Setters.Count - 1;
            }
        }

        private void HasCoordinates_IsCheckedChanged(object sender, RoutedEventArgs e)
        {
            if (_customSpawn != null && Setter.Items.Count > 0 && Spawn.Items.Count > 0 && !_changedProperty)
            {
                SetUnsavedChanges();
                if (HasCoordinates.IsChecked == true)
                {
                    XCoord.IsEnabled = true;
                    YCoord.IsEnabled = true;
                    ZCoord.IsEnabled = true;

                    var spawn = _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex];

                    if (spawn.Coordinates == null)
                    {
                        spawn.Coordinates = new IndividualSpawn.Position
                        {
                            X = double.Parse(XCoord.Text),
                            Y = double.Parse(YCoord.Text),
                            Z = double.Parse(ZCoord.Text)
                        };
                    }
                    else
                    {
                        spawn.Coordinates.X = double.Parse(XCoord.Text);
                        spawn.Coordinates.Y = double.Parse(YCoord.Text);
                        spawn.Coordinates.Z = double.Parse(ZCoord.Text);
                    }
                }
                else
                {
                    XCoord.IsEnabled = false;
                    YCoord.IsEnabled = false;
                    ZCoord.IsEnabled = false;

                    _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex].Coordinates = null;
                }
            }

            _changedProperty = false;
        }

        private void Spawn_SelectionChanged(object? sender, SelectionChangedEventArgs? e)
        {
            if (_customSpawn != null && !_changedProperty)
            {
                var spawn = _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex];

                SpawnChance.Text = spawn.Chance.ToString();
                SpawnArea.Text = spawn.Area.ToString();
                SubSpawn.Text = spawn.SubSpawn.ToString();
                if (spawn.Coordinates != null)
                {
                    _changedProperty = true;
                    HasCoordinates.IsChecked = true;
                    
                    var coords = spawn.Coordinates;

                    XCoord.IsEnabled = true;
                    YCoord.IsEnabled = true;
                    ZCoord.IsEnabled = true;

                    XCoord.Text = coords.X.ToString();
                    YCoord.Text = coords.Y.ToString();
                    ZCoord.Text = coords.Z.ToString();
                }
                else
                {
                    _changedProperty = true;
                    HasCoordinates.IsChecked = false;
                    
                    XCoord.IsEnabled = false;
                    YCoord.IsEnabled = false;
                    ZCoord.IsEnabled = false;

                    XCoord.Text = "0";
                    YCoord.Text = "0";
                    ZCoord.Text = "0";
                }
            }

            _changedProperty = false;
        }

        private void Setter_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (_customSpawn != null && !_changedProperty)
            {
                int idx = Setter.SelectedIndex;
                
                _changedProperty = true;
                Setter.Items.Clear();
                foreach (var _setter in _customSpawn.Setters)
                {
                    _changedProperty = true;
                    Setter.Items.Add(_setter.SetName);
                }

                _changedProperty = true;
                Setter.SelectedIndex = idx;

                var setter = _customSpawn.Setters[Setter.SelectedIndex];

                _changedProperty = true;
                SetName.Text = setter.SetName;
                Spawn.SelectedIndex = 0;
                Spawn_SelectionChanged(sender, null);
            }

            _changedProperty = false;
        }

        private void RemoveQuestID_Click(object sender, RoutedEventArgs e)
        {
            if (_customSpawn != null)
            {
                SetUnsavedChanges();
                QuestID.Text = "-1";
            }
        }

        private static bool IsValidNumber(TextBox textBox, TextCompositionEventArgs e, bool allowNegative = false)
        {
            if (allowNegative &&
                e.Text == "-" &&
                (textBox.Text.Length == 0 ||
                textBox.Text.Length == textBox.SelectedText.Length))
                return true;

            Regex regex = new(@"[^0-9]+");
            return !regex.IsMatch(e.Text);
        }

        private static bool IsValidFloatingPointNumber(TextBox textBox, TextCompositionEventArgs e)
        {
            if ((e.Text == "-" || e.Text == ".") &&
                (textBox.Text.Length == 0 ||
                textBox.Text.Length == textBox.SelectedText.Length))
                return true;

            Regex regex = new(@"^[0-9]*(\.[0-9]*)?$");
            return regex.IsMatch(e.Text);
        }

        private static string GetNewText(TextBox textBox, TextCompositionEventArgs e)
        {
            if (textBox.SelectedText.Length > 0)
            {
                return (textBox.Text + e.Text).Remove(textBox.SelectionStart, textBox.SelectionLength);
            }
            else
            {
                return textBox.Text + e.Text;
            }
        }

        private void ValidateIntTextBox(object sender, TextCompositionEventArgs e)
        {
            if (sender is TextBox textBox)
            {
                if (!IsValidNumber(textBox, e, true))
                {
                    e.Handled = true;
                    return;
                }

                string text = GetNewText(textBox, e);
                if (long.TryParse(text, out long n))
                {
                    if (n > int.MaxValue)
                    {
                        textBox.Text = int.MaxValue.ToString();
                        textBox.CaretIndex = textBox.Text.Length;
                        e.Handled = true;
                    }
                    else if (n < int.MinValue)
                    {
                        textBox.Text = int.MinValue.ToString();
                        textBox.CaretIndex = textBox.Text.Length;
                        e.Handled = true;
                    }
                }
            }
        }

        private void ValidateDoubleTextBox(object sender, TextCompositionEventArgs e)
        {
            if (sender is TextBox textBox)
            {
                if (!IsValidFloatingPointNumber(textBox, e))
                {
                    e.Handled = true;
                    return;
                }

                string text = GetNewText(textBox, e);
                if (double.TryParse(text, out double n))
                {
                    if (n > double.MaxValue)
                    {
                        textBox.Text = double.MaxValue.ToString();
                        textBox.CaretIndex = textBox.Text.Length;
                        e.Handled = true;
                    }
                    else if (n < double.MinValue)
                    {
                        textBox.Text = double.MinValue.ToString();
                        textBox.CaretIndex = textBox.Text.Length;
                        e.Handled = true;
                    }
                }
            }
        }

        private void SetName_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_customSpawn != null && !_changedProperty)
            {
                SetUnsavedChanges();

                _customSpawn.Setters[Setter.SelectedIndex].SetName = SetName.Text;
            }

            _changedProperty = false;
        }

        private void SpawnChance_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_customSpawn != null && !_changedProperty)
            {
                SetUnsavedChanges();
                if (int.TryParse(SpawnChance.Text, out int chance))
                {
                    _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex].Chance = chance;
                }
            }
        }

        private void SpawnArea_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_customSpawn != null)
            {
                SetUnsavedChanges();
                if (int.TryParse(SpawnArea.Text, out int area))
                {
                    _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex].Area = area;
                }
            }
        }

        private void SubSpawn_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_customSpawn != null)
            {
                SetUnsavedChanges();
                if (int.TryParse(SubSpawn.Text, out int subSpawn))
                {
                    _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex].SubSpawn = subSpawn;
                }
            }
        }

        private void XCoord_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_customSpawn != null)
            {
                SetUnsavedChanges();
                var coords = _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex].Coordinates;
                if (coords != null)
                {
                    if (double.TryParse(XCoord.Text, out double x))
                    {
                        coords.X = x;
                    }
                }
            }
        }

        private void YCoord_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_customSpawn != null)
            {
                SetUnsavedChanges();
                var coords = _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex].Coordinates;
                if (coords != null)
                {
                    if (double.TryParse(YCoord.Text, out double y))
                    {
                        coords.Y = y;
                    }
                }
            }
        }

        private void ZCoord_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_customSpawn != null)
            {
                SetUnsavedChanges();
                var coords = _customSpawn.Setters[Setter.SelectedIndex].Spawns[Spawn.SelectedIndex].Coordinates;
                if (coords != null)
                {
                    if (double.TryParse(ZCoord.Text, out double z))
                    {
                        coords.Z = z;
                    }
                }
            }
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            if (_hasUnsavedChanges)
            {
                var result = WarnUnsavedChanges();

                if (result == MessageBoxResult.Yes && _currentFile != "")
                {
                    SaveFile(null, null);
                }
                else if (result == MessageBoxResult.Cancel)
                {
                    e.Cancel = true;
                }
            }

            base.OnClosing(e);
        }

        private void Window_Drop(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                var files = (string[])e.Data.GetData(DataFormats.FileDrop);
                LoadFile(files[0]);
            }
        }

        private void AboutButton_Click(object sender, RoutedEventArgs e)
        {
            string about = $@"Monster Hunter Rise Spawn Editor
Made by Fexty.
To be used in conjunction with my Quest Editor.

Version: {Version}";

            MessageBox.Show(about, "About");
        }

        private void UsageButton_Click(object sender, RoutedEventArgs e)
        {
            System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo()
            {
                FileName = "https://github.com/mhvuze/MonsterHunterRiseModding/wiki/Custom-Spawn-Editing",
                UseShellExecute = true
            });
        }
    }
}
