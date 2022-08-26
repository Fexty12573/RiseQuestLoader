using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using Microsoft.Win32;
using Newtonsoft.Json;
using System.IO;
using System.Text.RegularExpressions;
using System.Media;
using System.ComponentModel;
using WPFCustomMessageBox;
using System.Collections.Generic;
using System.Windows.Media;
using System.Runtime.InteropServices;

namespace RiseQuestEditor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private CustomQuest? _customQuest;
        private string _currentFile;
        private bool _hasUnsavedChanges;
        private int _selectedLanguage;

        private static readonly string _version = "1.3.0";
        public static string Version => _version;

        public MainWindow()
        {
            InitializeComponent();

            QuestLevel.ItemsSource = EnumHelper.QuestLevel;
            MonsterLevel.ItemsSource = EnumHelper.EnemyLevel;
            Map.ItemsSource = EnumHelper.Map;
            BattleBgm.ItemsSource = EnumHelper.BattleBGMType;
            QuestClearBgm.ItemsSource = EnumHelper.ClearBGMType;
            QuestCondition1.ItemsSource = EnumHelper.QuestCondition;
            QuestCondition2.ItemsSource = EnumHelper.QuestCondition;

            Icon1.ItemsSource = EnumHelper.MonsterIconName;
            Icon2.ItemsSource = EnumHelper.MonsterIconName;
            Icon3.ItemsSource = EnumHelper.MonsterIconName;
            Icon4.ItemsSource = EnumHelper.MonsterIconName;
            Icon5.ItemsSource = EnumHelper.MonsterIconName;

            Objective1Type.ItemsSource = EnumHelper.QuestObjective;
            Objective2Type.ItemsSource = EnumHelper.QuestObjective;
            Objective1Monster.ItemsSource = EnumHelper.Monster;
            Objective2Monster.ItemsSource = EnumHelper.Monster;
            Objective1Item.ItemsSource = EnumHelper.Item;
            Objective2Item.ItemsSource = EnumHelper.Item;

            SwapStopType.ItemsSource = EnumHelper.SwapStopType;
            SwapExecType.ItemsSource = EnumHelper.SwapExecType;
            SwapCondition1.ItemsSource = EnumHelper.SwapCondition;
            SwapCondition2.ItemsSource = EnumHelper.SwapCondition;

            for (int i = 0; i < 7; i++)
            {
                ComboBox id = (ComboBox)FindName($"Monster{i + 1}Id");
                ComboBox hp = (ComboBox)FindName($"Monster{i + 1}Hp");
                ComboBox atk = (ComboBox)FindName($"Monster{i + 1}Attack");
                ComboBox part = (ComboBox)FindName($"Monster{i + 1}PartHp");
                ComboBox other = (ComboBox)FindName($"Monster{i + 1}Other");
                ComboBox multi = (ComboBox)FindName($"Monster{i + 1}Multi");
                ComboBox dif = (ComboBox)FindName($"Monster{i + 1}Difficulty");
                ComboBox cond = (ComboBox)FindName($"Monster{i + 1}SpawnCondition");
                ComboBox type = (ComboBox)FindName($"Monster{i + 1}IndividualType");

                id.ItemsSource = EnumHelper.Monster;
                hp.ItemsSource = EnumHelper.HealthRate;
                atk.ItemsSource = EnumHelper.AttackRate;
                part.ItemsSource = EnumHelper.PartsRate;
                other.ItemsSource = EnumHelper.OtherRate;
                multi.ItemsSource = EnumHelper.MultiRate;
                dif.ItemsSource = EnumHelper.NandoYuragi;
                cond.ItemsSource = EnumHelper.SpawnCondition;
                type.ItemsSource = EnumHelper.IndividualType;

                other.DisplayMemberPath = "Key";
                multi.DisplayMemberPath = "Key";
            }

            SmallMonSpawnType.ItemsSource = EnumHelper.EmsSetNo;
            SmallMonHp.ItemsSource = EnumHelper.HealthRate;
            SmallMonAttack.ItemsSource = EnumHelper.AttackRate;
            SmallMonOther.ItemsSource = EnumHelper.OtherRate;
            SmallMonPartHp.ItemsSource = EnumHelper.PartsRate;
            SmallMonMulti.ItemsSource = EnumHelper.MultiRate;

            SmallMonOther.DisplayMemberPath = "Key";
            SmallMonMulti.DisplayMemberPath = "Key";

            RampageQuestLevel.ItemsSource = EnumHelper.QuestLevel;
            RampageQuestMap.ItemsSource = EnumHelper.Map;
            RampageQuestCategory.ItemsSource = EnumHelper.RampageCategory;

            RampageSubTarget1.ItemsSource = EnumHelper.QuestObjective;
            RampageSubTarget2.ItemsSource = EnumHelper.QuestObjective;
            RampageSubTarget3.ItemsSource = EnumHelper.QuestObjective;
            RampageSubTarget4.ItemsSource = EnumHelper.QuestObjective;
            RampageSubTarget5.ItemsSource = EnumHelper.QuestObjective;
            RampageSubTarget6.ItemsSource = EnumHelper.QuestObjective;

            Wave1BossMonster.ItemsSource = EnumHelper.Monster;
            Wave1Monster1.ItemsSource = EnumHelper.Monster;
            Wave1Monster2.ItemsSource = EnumHelper.Monster;
            Wave1Monster3.ItemsSource = EnumHelper.Monster;
            Wave1Monster4.ItemsSource = EnumHelper.Monster;

            Wave2BossMonster.ItemsSource = EnumHelper.Monster;
            Wave2Monster1.ItemsSource = EnumHelper.Monster;
            Wave2Monster2.ItemsSource = EnumHelper.Monster;
            Wave2Monster3.ItemsSource = EnumHelper.Monster;
            Wave2Monster4.ItemsSource = EnumHelper.Monster;

            Wave3BossMonster.ItemsSource = EnumHelper.Monster;
            Wave3Monster1.ItemsSource = EnumHelper.Monster;
            Wave3Monster2.ItemsSource = EnumHelper.Monster;
            Wave3Monster3.ItemsSource = EnumHelper.Monster;
            Wave3Monster4.ItemsSource = EnumHelper.Monster;

            GameLanguage.ItemsSource = EnumHelper.Languages;
            FallbackLanguage.ItemsSource = EnumHelper.Languages;

            GameLanguage.DisplayMemberPath = "Value.Name";
            FallbackLanguage.DisplayMemberPath = "Value.Name";

            RemoveUnsavedChanges();

            _currentFile = "";
            _selectedLanguage = 0;
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

        protected override void OnInitialized(EventArgs e)
        {
            base.OnInitialized(e);
            RemoveUnsavedChanges();
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

            _customQuest = new CustomQuest();
            PopulateEditorFields();

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
                string path = Path.GetDirectoryName(dlg.FileName)!;
                if (!Directory.Exists(path))
                {
                    Directory.CreateDirectory(path);
                }

                _currentFile = dlg.FileName;
                Title = "Monster Hunter Rise Quest Editor - " + System.IO.Path.GetFileName(_currentFile);

                using StreamWriter writer = new(_currentFile, false, Encoding.UTF8);
                writer.Write(JsonConvert.SerializeObject(_customQuest, Formatting.Indented, new ByteArrayJsonConverter()));
                RemoveUnsavedChanges();
            }
        }

        private void OpenFile(object sender, ExecutedRoutedEventArgs e)
        {
            OpenFileDialog dlg = new()
            {
                CheckFileExists = true,
                Title = "Open Quest File",
                Filter = "Quest Files | *.json"
            };

            if (dlg.ShowDialog() == true)
            {
                LoadFile(dlg.FileName);
            }
        }

        private void SaveFile(object? sender, ExecutedRoutedEventArgs? e)
        {
            if (_customQuest != null)
            {
                if (_currentFile != "")
                {
                    StoreEditorFields();

                    using StreamWriter writer = new(_currentFile, false, Encoding.UTF8);
                    writer.Write(JsonConvert.SerializeObject(_customQuest, Formatting.Indented, new ByteArrayJsonConverter()));
                    RemoveUnsavedChanges();
                }
                else
                {
                    SaveFileAs(sender, e);
                }
            }
            else
            {
                MessageBox.Show("Open a Quest first", "Can't Save", MessageBoxButton.OK, MessageBoxImage.Information);
            }
        }

        private void SaveFileAs(object? sender, ExecutedRoutedEventArgs? e)
        {
            if (_customQuest != null)
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
                    Title = "Monster Hunter Rise Quest Editor - " + System.IO.Path.GetFileName(_currentFile);

                    SaveFile(sender, e);
                }
            }
            else
            {
                MessageBox.Show("Open a Quest first", "Can't Save", MessageBoxButton.OK, MessageBoxImage.Information);
            }
        }

        public void LoadFile(string path)
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
            _customQuest = JsonConvert.DeserializeObject<CustomQuest>(sr.ReadToEnd());

            if (_customQuest != null)
            {
                _currentFile = path;
                Title = "Monster Hunter Rise Quest Editor - " + System.IO.Path.GetFileName(_currentFile);

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

        private void ExitProgram(object sender, ExecutedRoutedEventArgs e)
        {
            Close();
        }

        private void PopulateEditorFields()
        {
            QuestID.Text = _customQuest!.QuestID.ToString();

            if (_customQuest!.QuestData != null)
            {
                QuestData data = _customQuest!.QuestData;

                QuestLevel.SelectedValue = (int)data.QuestLevel;
                MonsterLevel.SelectedValue = (int)data.EnemyLevel;
                Map.SelectedValue = (int)data.Map;
                QuestTime.Text = data.BaseTime.ToString();
                TimeLimit.Text = data.TimeLimit.ToString();
                Carts.Text = data.Carts.ToString();
                BattleBgm.SelectedValue = data.BattleBGMType;
                QuestClearBgm.SelectedValue = data.ClearBGMType;

                QuestCondition1.SelectedValue = (int)data.QuestConditions[0];
                QuestCondition2.SelectedValue = (int)data.QuestConditions[1];

                Money.Text = data.Reward.Zenny.ToString();
                VillagePoints.Text = data.Reward.Points.ToString();
                HrPoints.Text = data.Reward.HRP.ToString();
                SupplyTable.Text = data.SupplyTable.ToString();

                Icon1.SelectedValue = data.Icons[0];
                Icon2.SelectedValue = data.Icons[1];
                Icon3.SelectedValue = data.Icons[2];
                Icon4.SelectedValue = data.Icons[3];
                Icon5.SelectedValue = data.Icons[4];

                Objective1Type.SelectedValue = (int)data.TargetTypes[0];
                Objective2Type.SelectedValue = (int)data.TargetTypes[1];

                Objective1Monster.SelectedValue = data.TargetMonsters[0];
                Objective2Monster.SelectedValue = data.TargetMonsters[1];

                Objective1Item.SelectedValue = data.TargetItemIds[0];
                Objective2Item.SelectedValue = data.TargetItemIds[1];

                Objective1Amount.Text = data.TargetAmounts[0].ToString();
                Objective2Amount.Text = data.TargetAmounts[1].ToString();

                HuntingQuest.IsChecked = data.QuestType.HasFlag(QuestType.HUNTING);
                KillQuest.IsChecked = data.QuestType.HasFlag(QuestType.KILL);
                CaptureQuest.IsChecked = data.QuestType.HasFlag(QuestType.CAPTURE);
                BossRushQuest.IsChecked = data.QuestType.HasFlag(QuestType.BOSSRUSH);
                GatheringQuest.IsChecked = data.QuestType.HasFlag(QuestType.COLLECTS);
                TourQuest.IsChecked = data.QuestType.HasFlag(QuestType.TOUR);
                ArenaQuest.IsChecked = data.QuestType.HasFlag(QuestType.ARENA);
                SpecialQuest.IsChecked = data.QuestType.HasFlag(QuestType.SPECIAL);
                RampageQuest.IsChecked = data.QuestType.HasFlag(QuestType.HYAKURYU);
                TrainingQuest.IsChecked = data.QuestType.HasFlag(QuestType.TRAINING);
                FollowerQuest.IsChecked = data.QuestType.HasFlag(QuestType.KYOUSEI);

                if (_customQuest.EnemyData != null)
                {
                    EnemyData enemy = _customQuest.EnemyData;

                    for (int i = 0; i < 7; i++)
                    {
                        ((ComboBox)FindName($"Monster{i + 1}Id")).SelectedValue = data.Monsters[i].Id;
                        ((ComboBox)FindName($"Monster{i + 1}Hp")).SelectedValue = enemy.Monsters[i].HealthTable;
                        ((ComboBox)FindName($"Monster{i + 1}Attack")).SelectedValue = enemy.Monsters[i].AttackTable;
                        ((ComboBox)FindName($"Monster{i + 1}PartHp")).SelectedValue = enemy.Monsters[i].PartTable;
                        ((ComboBox)FindName($"Monster{i + 1}Other")).SelectedValue = (int)enemy.Monsters[i].OtherTable;
                        ((ComboBox)FindName($"Monster{i + 1}Multi")).SelectedValue = (int)enemy.Monsters[i].MultiTable;
                        ((ComboBox)FindName($"Monster{i + 1}Difficulty")).SelectedValue = (int)enemy.Monsters[i].Difficulty;
                        ((ComboBox)FindName($"Monster{i + 1}SpawnCondition")).SelectedValue = (int)data.Monsters[i].SpawnCondition;
                        ((ComboBox)FindName($"Monster{i + 1}IndividualType")).SelectedValue = (int)enemy.Monsters[i].IndividualType;
                        ((TextBox)FindName($"Monster{i + 1}SubType")).Text = enemy.Monsters[i].SubType.ToString();
                        ((TextBox)FindName($"Monster{i + 1}SetName")).Text = enemy.Monsters[i].SetName.ToString();
                        ((TextBox)FindName($"Monster{i + 1}Stamina")).Text = enemy.Monsters[i].StaminaTable.ToString();
                        ((TextBox)FindName($"Monster{i + 1}SpawnParam")).Text = data.Monsters[i].SpawnParam.ToString();
                        ((TextBox)FindName($"Monster{i + 1}SizeTable")).Text = enemy.Monsters[i].SizeTable.ToString();
                        ((TextBox)FindName($"Monster{i + 1}Size")).Text = enemy.Monsters[i].Size.ToString();
                    }

                    SmallMonSpawnType.SelectedValue = (int)enemy.SmallMonsters.SpawnType;
                    SmallMonAttack.SelectedValue = enemy.SmallMonsters.AttackTable;
                    SmallMonHp.SelectedValue = enemy.SmallMonsters.HealthTable;
                    SmallMonPartHp.SelectedValue = enemy.SmallMonsters.PartTable;
                    SmallMonOther.SelectedValue = (int)enemy.SmallMonsters.OtherTable;
                    SmallMonMulti.SelectedValue = (int)enemy.SmallMonsters.MultiTable;
                }

                ExtraMonsterCount.Text = data.ExtraMonsterCount.ToString();
                SwapStopType.SelectedValue = (int)data.SwapStopType;
                SwapStopParam.Text = data.SwapStopParam.ToString();
                SwapExitRide.IsChecked = data.SwapExitRide;
                SwapExecType.SelectedValue = (int)data.SwapExecType;

                SwapFrequency1.Text = data.SwapFrequencies[0].ToString();
                SwapCondition1.SelectedValue = (int)data.SwapConditions[0];
                SwapParam1.Text = data.SwapParams[0].ToString();
                SwapExitTime1.Text = data.SwapExitTimes[0].ToString();

                SwapFrequency2.Text = data.SwapFrequencies[1].ToString();
                SwapCondition2.SelectedValue = (int)data.SwapConditions[1];
                SwapParam2.Text = data.SwapParams[1].ToString();
                SwapExitTime2.Text = data.SwapExitTimes[1].ToString();

                IsTutorial.IsChecked = data.Tutorial;
                IsFromNpc.IsChecked = data.FromNpc;

                FenceDefaultActive.IsChecked = data.ArenaParam.FenceDefaultActive;
                FenceUpTime.Text = data.ArenaParam.FenceUptime.ToString();
                FenceInitialDelay.Text = data.ArenaParam.FenceInitialDelay.ToString();
                FenceCooldown.Text = data.ArenaParam.FenceCooldown.ToString();

                ArenaPillar1.IsChecked = data.ArenaParam.Pillars[0];
                ArenaPillar2.IsChecked = data.ArenaParam.Pillars[1];
                ArenaPillar3.IsChecked = data.ArenaParam.Pillars[2];
            }

            if (_customQuest.QuestText != null)
            {
                QuestText text = _customQuest.QuestText;

                for (int i = 0; i < text.QuestInfo.Count; ++i)
                {
                    var info = text.QuestInfo[i];

                    if (info.Fail == null)
                    {
                        info.Fail = "";
                        text.QuestInfo[i] = info;
                    }

                    if (info.Language == text.FallbackLanguage)
                    {
                        QuestName.Text = info.Name;
                        QuestClient.Text = info.Client;
                        QuestDesc.Text = info.Description;
                        QuestTarget.Text = info.Target;
                        QuestFail.Text = info.Fail;
                    }
                }

                int index = EnumHelper.GetLanguageIndex(text.FallbackLanguage!);
                _selectedLanguage = index;
                GameLanguage.SelectedIndex = index;
                FallbackLanguage.SelectedIndex = index;
            }

            if (_customQuest.RampageData != null)
            {
                RampageData data = _customQuest.RampageData;

                RampageSeed.Text = data.Seed.ToString();

                RampageFixWaveOrder.IsChecked = data.QuestAttr.HasFlag(RampageAttr.FixWaveOrder);
                RampageLotHighEx.IsChecked = data.QuestAttr.HasFlag(RampageAttr.Lot_HighEx);
                RampageLotTrueEd.IsChecked = data.QuestAttr.HasFlag(RampageAttr.Lot_TrueED);
                RampageFinalBossKill.IsChecked = data.QuestAttr.HasFlag(RampageAttr.FinalBossKill);

                RampageQuestLevel.SelectedValue = (int)data.QuestLevel;
                RampageQuestMap.SelectedValue = (int)data.Map;
                RampageQuestCategory.SelectedValue = (int)data.Category;

                RampageQuestBaseTime.Text = data.BaseTime.ToString();
                RampageStartBlock.Text = data.StartBlock.ToString();
                RampageEndBlock.Text = data.EndBlock.ToString();
                RampageExtraWaveCount.Text = data.ExtraWaveCount.ToString();

                RampageWeaponUnlockTable.Text = data.WeaponUnlockTable.ToString();
                RampageApexOrderTable.Text = data.ApexOrderTable.ToString();
                RampageExtraMonsterNandoTable.Text = data.ExtraMonsterNandoTable.ToString();
                RampageSubTarget5Wave.Text = data.SubTarget5Wave.ToString();
                RampageIsVillage.IsChecked = data.IsVillage;

                RampageSubTarget1.SelectedValue = (int)data.SubTargets[0];
                RampageSubTarget2.SelectedValue = (int)data.SubTargets[1];
                RampageSubTarget3.SelectedValue = (int)data.SubTargets[2];
                RampageSubTarget4.SelectedValue = (int)data.SubTargets[3];
                RampageSubTarget5.SelectedValue = (int)data.SubTargets[4];
                RampageSubTarget6.SelectedValue = (int)data.SubTargets[5];

                for (int i = 0; i < 3; i++)
                {
                    ((ComboBox)FindName($"Wave{i + 1}BossMonster")).SelectedValue = (int)data.Waves[i].BossMonster;
                    ((TextBox)FindName($"Wave{i + 1}BossSubType")).Text = data.Waves[i].BossSubType.ToString();
                    ((TextBox)FindName($"Wave{i + 1}OrderTable")).Text = data.Waves[i].OrderTable.ToString();
                    ((TextBox)FindName($"Wave{i + 1}BossMonsterDifficulty")).Text = data.Waves[i].BossMonsterNandoTable.ToString();
                    ((TextBox)FindName($"Wave{i + 1}WaveMonsterDifficulty")).Text = data.Waves[i].WaveMonsterNandoTable.ToString();


                    ((ComboBox)FindName($"Wave{i + 1}Monster1")).SelectedValue = data.Waves[i].Monsters[0];
                    ((ComboBox)FindName($"Wave{i + 1}Monster2")).SelectedValue = data.Waves[i].Monsters[1];
                    ((ComboBox)FindName($"Wave{i + 1}Monster3")).SelectedValue = data.Waves[i].Monsters[2];
                    ((ComboBox)FindName($"Wave{i + 1}Monster4")).SelectedValue = data.Waves[i].Monsters[3];
                }

                RampageTab.IsEnabled = true;
                RampageWavesTab.IsEnabled = true;
            }
            else
            {
                MainTabs.SelectedIndex = 0;
                RampageTab.IsEnabled = false;
                RampageWavesTab.IsEnabled = false;
            }
        }

        private void StoreEditorFields()
        {
            if (_customQuest == null)
            {
                return;
            }

            _customQuest.QuestID = int.Parse(QuestID.Text);

            if (_customQuest.QuestData != null)
            {
                QuestData data = _customQuest.QuestData!;

                data.QuestLevel = (QuestLevel)QuestLevel.SelectedValue;
                data.EnemyLevel = (EnemyLv)MonsterLevel.SelectedValue;
                data.Map = (MapNoType)Map.SelectedValue;
                data.BaseTime = uint.Parse(QuestTime.Text);
                data.TimeLimit = uint.Parse(TimeLimit.Text);
                data.Carts = uint.Parse(Carts.Text);
                data.BattleBGMType = (int)BattleBgm.SelectedValue;
                data.ClearBGMType = (int)QuestClearBgm.SelectedValue;
                data.QuestConditions[0] = (QuestOrderType)QuestCondition1.SelectedValue;
                data.QuestConditions[1] = (QuestOrderType)QuestCondition2.SelectedValue;

                data.Reward = new QuestData.Rewards
                {
                    Zenny = uint.Parse(Money.Text),
                    Points = uint.Parse(VillagePoints.Text),
                    HRP = uint.Parse(HrPoints.Text)
                };
                data.SupplyTable = uint.Parse(SupplyTable.Text);

                data.Icons[0] = (int)Icon1.SelectedValue;
                data.Icons[1] = (int)Icon2.SelectedValue;
                data.Icons[2] = (int)Icon3.SelectedValue;
                data.Icons[3] = (int)Icon4.SelectedValue;
                data.Icons[4] = (int)Icon5.SelectedValue;

                data.TargetTypes[0] = (QuestTargetType)Convert.ToInt32(Objective1Type.SelectedValue);
                data.TargetTypes[1] = (QuestTargetType)Convert.ToInt32(Objective2Type.SelectedValue);

                data.TargetMonsters[0] = Convert.ToUInt32(Objective1Monster.SelectedValue);
                data.TargetMonsters[1] = Convert.ToUInt32(Objective2Monster.SelectedValue);

                data.TargetItemIds[0] = Convert.ToUInt32(Objective1Item.SelectedValue);
                data.TargetItemIds[1] = Convert.ToUInt32(Objective2Item.SelectedValue);

                data.TargetAmounts[0] = uint.Parse(Objective1Amount.Text);
                data.TargetAmounts[1] = uint.Parse(Objective2Amount.Text);

                data.QuestType = QuestType.INVALID;
                data.QuestType |= (bool)HuntingQuest.IsChecked! ? QuestType.HUNTING : 0;
                data.QuestType |= (bool)KillQuest.IsChecked! ? QuestType.KILL : 0;
                data.QuestType |= (bool)CaptureQuest.IsChecked! ? QuestType.CAPTURE : 0;
                data.QuestType |= (bool)BossRushQuest.IsChecked! ? QuestType.BOSSRUSH : 0;
                data.QuestType |= (bool)GatheringQuest.IsChecked! ? QuestType.COLLECTS : 0;
                data.QuestType |= (bool)TourQuest.IsChecked! ? QuestType.TOUR : 0;
                data.QuestType |= (bool)ArenaQuest.IsChecked! ? QuestType.ARENA : 0;
                data.QuestType |= (bool)SpecialQuest.IsChecked! ? QuestType.SPECIAL : 0;
                data.QuestType |= (bool)RampageQuest.IsChecked! ? QuestType.HYAKURYU : 0;
                data.QuestType |= (bool)TrainingQuest.IsChecked! ? QuestType.TRAINING : 0;
                data.QuestType |= (bool)FollowerQuest.IsChecked! ? QuestType.KYOUSEI : 0;

                if (_customQuest.EnemyData != null)
                {
                    EnemyData enemy = _customQuest.EnemyData;

                    for (int i = 0; i < 7; i++)
                    {
                        data.Monsters[i].Id = Convert.ToUInt32(((ComboBox)FindName($"Monster{i + 1}Id")).SelectedValue);
                        enemy.Monsters[i].HealthTable = Convert.ToUInt16(((ComboBox)FindName($"Monster{i + 1}Hp")).SelectedValue);
                        enemy.Monsters[i].AttackTable = Convert.ToUInt16(((ComboBox)FindName($"Monster{i + 1}Attack")).SelectedValue);
                        enemy.Monsters[i].PartTable = Convert.ToUInt16(((ComboBox)FindName($"Monster{i + 1}PartHp")).SelectedValue);
                        enemy.Monsters[i].OtherTable = Convert.ToUInt16(((ComboBox)FindName($"Monster{i + 1}Other")).SelectedValue);
                        enemy.Monsters[i].MultiTable = Convert.ToByte(((ComboBox)FindName($"Monster{i + 1}Multi")).SelectedValue);
                        enemy.Monsters[i].Difficulty = (NandoYuragi)(int)((ComboBox)FindName($"Monster{i + 1}Difficulty")).SelectedValue;
                        data.Monsters[i].SpawnCondition = (BossSetCondition)(int)((ComboBox)FindName($"Monster{i + 1}SpawnCondition")).SelectedValue;
                        enemy.Monsters[i].IndividualType = (EnemyIndividualType)(int)((ComboBox)FindName($"Monster{i + 1}IndividualType")).SelectedValue;
                        enemy.Monsters[i].SubType = byte.Parse(((TextBox)FindName($"Monster{i + 1}SubType")).Text);
                        enemy.Monsters[i].SetName = ((TextBox)FindName($"Monster{i + 1}SetName")).Text;
                        enemy.Monsters[i].StaminaTable = byte.Parse(((TextBox)FindName($"Monster{i + 1}Stamina")).Text);
                        data.Monsters[i].SpawnParam = uint.Parse(((TextBox)FindName($"Monster{i + 1}SpawnParam")).Text);
                        enemy.Monsters[i].SizeTable = int.Parse(((TextBox)FindName($"Monster{i + 1}SizeTable")).Text);
                        enemy.Monsters[i].Size = byte.Parse(((TextBox)FindName($"Monster{i + 1}Size")).Text);
                    }

                    enemy.SmallMonsters = new EnemyData.SmallMonsterData
                    {
                        SpawnType = (EmsSetNo)(int)SmallMonSpawnType.SelectedValue,
                        AttackTable = Convert.ToByte(SmallMonAttack.SelectedValue),
                        HealthTable = Convert.ToByte(SmallMonHp.SelectedValue),
                        PartTable = Convert.ToByte(SmallMonPartHp.SelectedValue),
                        OtherTable = Convert.ToByte(SmallMonOther.SelectedValue),
                        MultiTable = Convert.ToByte(SmallMonMulti.SelectedValue)
                    };
                }

                data.ExtraMonsterCount = uint.Parse(ExtraMonsterCount.Text);
                data.SwapStopType = (SwapStopType)(int)SwapStopType.SelectedValue;
                data.SwapStopParam = byte.Parse(SwapStopParam.Text);
                data.SwapExitRide = (bool)SwapExitRide.IsChecked!;
                data.SwapExecType = (SwapExecType)(int)SwapExecType.SelectedValue;

                data.SwapFrequencies[0] = byte.Parse(SwapFrequency1.Text);
                data.SwapConditions[0] = (SwapSetCondition)(int)SwapCondition1.SelectedValue;
                data.SwapParams[0] = byte.Parse(SwapParam1.Text);
                data.SwapExitTimes[0] = byte.Parse(SwapExitTime1.Text);

                data.SwapFrequencies[1] = byte.Parse(SwapFrequency2.Text);
                data.SwapConditions[1] = (SwapSetCondition)(int)SwapCondition2.SelectedValue;
                data.SwapParams[1] = byte.Parse(SwapParam2.Text);
                data.SwapExitTimes[1] = byte.Parse(SwapExitTime2.Text);

                data.Tutorial = IsTutorial.IsChecked == true;
                data.FromNpc = IsFromNpc.IsChecked == true;

                data.ArenaParam = new QuestData.ArenaParameters
                {
                    FenceDefaultActive = (bool)FenceDefaultActive.IsChecked!,
                    FenceUptime = ushort.Parse(FenceUpTime.Text),
                    FenceInitialDelay = ushort.Parse(FenceInitialDelay.Text),
                    FenceCooldown = ushort.Parse(FenceCooldown.Text),
                    Pillars = new bool[3]
                    {
                        ArenaPillar1.IsChecked == true,
                        ArenaPillar1.IsChecked == true,
                        ArenaPillar1.IsChecked == true
                    }
                };

                _customQuest.QuestData = data;
            }

            if (_customQuest.QuestText != null)
            {
                QuestText text = _customQuest.QuestText;

                for (int i = 0; i < text.QuestInfo.Count; i++)
                {
                    var info = text.QuestInfo[i];
                    if (info.Language == EnumHelper.Languages[GameLanguage.SelectedIndex].Identifier)
                    {
                        info.Name = QuestName.Text;
                        info.Client = QuestClient.Text;
                        info.Description = QuestDesc.Text;
                        info.Target = QuestTarget.Text;
                        info.Fail = QuestFail.Text;
                        text.QuestInfo[i] = info;

                        break;
                    }
                }

                text.FallbackLanguage = EnumHelper.Languages[FallbackLanguage.SelectedIndex].Identifier;

                List<int> toRemove = new();
                for (int i = 0; i < text.QuestInfo.Count; i++)
                {
                    var info = text.QuestInfo[i];
                    if (info.Name == "" && info.Client == "" && info.Description == "" && info.Target == "")
                        toRemove.Add(i);
                }

                foreach (int n in toRemove)
                    text.QuestInfo.RemoveAt(n);
            }

            if (_customQuest.RampageData != null)
            {
                RampageData data = _customQuest.RampageData;

                data.Seed = int.Parse(RampageSeed.Text);

                data.QuestAttr = RampageAttr.None;
                data.QuestAttr |= (RampageFixWaveOrder.IsChecked == true) ? RampageAttr.FixWaveOrder : 0;
                data.QuestAttr |= (RampageLotHighEx.IsChecked == true) ? RampageAttr.Lot_HighEx : 0;
                data.QuestAttr |= (RampageLotTrueEd.IsChecked == true) ? RampageAttr.Lot_TrueED : 0;
                data.QuestAttr |= (RampageFinalBossKill.IsChecked == true) ? RampageAttr.FinalBossKill : 0;

                data.QuestLevel = (QuestLevel)RampageQuestLevel.SelectedValue;
                data.Map = (MapNoType)RampageQuestMap.SelectedValue;
                data.Category = (RampageCategory)RampageQuestCategory.SelectedValue;

                data.BaseTime = uint.Parse(RampageQuestBaseTime.Text);
                data.StartBlock = byte.Parse(RampageStartBlock.Text);
                data.EndBlock = byte.Parse(RampageEndBlock.Text);
                data.ExtraWaveCount = byte.Parse(RampageExtraWaveCount.Text);

                data.WeaponUnlockTable = byte.Parse(RampageWeaponUnlockTable.Text);
                data.ApexOrderTable = byte.Parse(RampageApexOrderTable.Text);
                data.ExtraMonsterNandoTable = sbyte.Parse(RampageExtraMonsterNandoTable.Text);
                data.SubTarget5Wave = byte.Parse(RampageSubTarget5Wave.Text);
                data.IsVillage = RampageIsVillage.IsChecked == true;

                data.SubTargets[0] = (QuestTargetType)(int)RampageSubTarget1.SelectedValue;
                data.SubTargets[1] = (QuestTargetType)(int)RampageSubTarget2.SelectedValue;
                data.SubTargets[2] = (QuestTargetType)(int)RampageSubTarget3.SelectedValue;
                data.SubTargets[3] = (QuestTargetType)(int)RampageSubTarget4.SelectedValue;
                data.SubTargets[4] = (QuestTargetType)(int)RampageSubTarget5.SelectedValue;
                data.SubTargets[5] = (QuestTargetType)(int)RampageSubTarget6.SelectedValue;


                for (int i = 0; i < 3; i++)
                {
                    data.Waves[i].BossMonster = Convert.ToUInt32(((ComboBox)FindName($"Wave{i + 1}BossMonster")).SelectedValue);
                    data.Waves[i].BossSubType = Convert.ToUInt32(((TextBox)FindName($"Wave{i + 1}BossSubType")).Text);
                    data.Waves[i].OrderTable = Convert.ToInt32(((TextBox)FindName($"Wave{i + 1}OrderTable")).Text);
                    data.Waves[i].BossMonsterNandoTable = Convert.ToInt32(((TextBox)FindName($"Wave{i + 1}BossMonsterDifficulty")).Text);
                    data.Waves[i].WaveMonsterNandoTable = Convert.ToInt32(((TextBox)FindName($"Wave{i + 1}WaveMonsterDifficulty")).Text);

                    data.Waves[i].Monsters[0] = Convert.ToUInt32(((ComboBox)FindName($"Wave{i + 1}Monster1")).SelectedValue);
                    data.Waves[i].Monsters[1] = Convert.ToUInt32(((ComboBox)FindName($"Wave{i + 1}Monster2")).SelectedValue);
                    data.Waves[i].Monsters[2] = Convert.ToUInt32(((ComboBox)FindName($"Wave{i + 1}Monster3")).SelectedValue);
                    data.Waves[i].Monsters[3] = Convert.ToUInt32(((ComboBox)FindName($"Wave{i + 1}Monster4")).SelectedValue);
                }
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

        private void ValidateNumberTextBox(object sender, TextCompositionEventArgs e)
        {
            if (sender is TextBox textBox)
            {
                e.Handled = !IsValidNumber(textBox, e, false);
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

        private void ValidateSByteTextBox(object sender, TextCompositionEventArgs e)
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
                    if (n > sbyte.MaxValue)
                    {
                        textBox.Text = sbyte.MaxValue.ToString();
                        e.Handled = true;
                    }
                    else if (n < sbyte.MinValue)
                    {
                        textBox.Text = sbyte.MinValue.ToString();
                        e.Handled = true;
                    }
                }
            }
        }

        private void ValidateByteTextBox(object sender, TextCompositionEventArgs e)
        {
            if (sender is TextBox textBox)
            {
                if (!IsValidNumber(textBox, e, false))
                {
                    e.Handled = true;
                    return;
                }

                string text = GetNewText(textBox, e);
                if (ulong.TryParse(text, out ulong n))
                {
                    if (n > 255)
                    {
                        textBox.Text = "255";
                        e.Handled = true;
                    }
                }
            }
        }

        private void ValidateUShortTextBox(object sender, TextCompositionEventArgs e)
        {
            if (sender is TextBox textBox)
            {
                if (!IsValidNumber(textBox, e, false))
                {
                    e.Handled = true;
                    return;
                }

                string text = GetNewText(textBox, e);
                if (ulong.TryParse(text, out ulong n))
                {
                    if (n > ushort.MaxValue)
                    {
                        textBox.Text = ushort.MaxValue.ToString();
                        e.Handled = true;
                    }
                }
            }
        }

        private void IconChanged(object sender, SelectionChangedEventArgs e)
        {
            if (sender is ComboBox icon)
            {
                if (icon.SelectedValue != null && (int)icon.SelectedValue != 999)
                {
                    var name = (string)icon.GetValue(NameProperty);
                    Image image = (Image)FindName($"IconImage{name.Last()}");

                    Uri uri = new(EnumHelper.MonsterIconFile[(int)icon.SelectedValue], UriKind.Relative);
                    image.Source = new BitmapImage(uri);
                }
                else
                {
                    var name = (string)icon.GetValue(NameProperty);
                    Image image = (Image)FindName($"IconImage{name.Last()}");
                    image.Source = null;
                }
            }
        }

        private void SmallMonOtherOrMultiChanged(object sender, SelectionChangedEventArgs e)
        {
            var other = EnumHelper.OtherRate[(int)SmallMonOther.SelectedValue];
            SmallMonInfo.Text =
$@"Other:
Defense: {other.Defense}
ElementA: {other.ElementA}
ElementB: {other.ElementB}
Stun: {other.Stun}
Exhaust: {other.Exhaust}
Wyvern Riding: {other.WyvernRide}";
        }

        private void MonsterOtherChanged(object sender, SelectionChangedEventArgs e)
        {
            if (sender is ComboBox box)
            {
                var other = EnumHelper.OtherRate[(int)box.SelectedValue];
                var name = (string)box.GetValue(NameProperty);
                TextBlock textBlock = (TextBlock)FindName($"Monster{name[7]}Info");

                textBlock.Text =
$@"Other Stats:
Defense: {other.Defense}
ElementA: {other.ElementA}
ElementB: {other.ElementB}
Stun: {other.Stun}
Exhaust: {other.Exhaust}
Wyvern Riding: {other.WyvernRide}";
            }
        }

        private void UsageButton_Click(object sender, RoutedEventArgs e)
        {
            System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo()
            {
                FileName = "https://github.com/mhvuze/MonsterHunterRiseModding/wiki/Quest-Editor-Field-Breakdown",
                UseShellExecute = true
            });
        }

        private void AboutButton_Click(object sender, RoutedEventArgs e)
        {
            string about = $@"Monster Hunter Rise Quest Editor
Made by Fexty

Version: {Version}

Thanks:
- DSC-173 for help with understanding the quest fields.
- praydog for developing REFramework which helped greatly in understanding quest structure.
- Aradi147 for his MHW Quest Editor, which I took heavy inspiration from.
- Everyone who helped testing";

            MessageBox.Show(about, "About");
        }

        private void Property_ContentChanged(object sender, RoutedEventArgs e)
        {
            SetUnsavedChanges();
        }

        private void Window_Drop(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                var files = (string[])e.Data.GetData(DataFormats.FileDrop);
                LoadFile(files[0]);
            }
        }

        private void GameLanguage_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (_customQuest != null)
            {
                var text = _customQuest.QuestText!;
                for (int i = 0; i < text.QuestInfo.Count; i++)
                {
                    var info = text.QuestInfo[i];
                    if (info.Language == EnumHelper.Languages[_selectedLanguage].Identifier)
                    {
                        info.Name = QuestName.Text;
                        info.Client = QuestClient.Text;
                        info.Description = QuestDesc.Text;
                        info.Target = QuestTarget.Text;
                        info.Fail = QuestFail.Text;
                        text.QuestInfo[i] = info;

                        break;
                    }
                }

                int newIndex = GameLanguage.SelectedIndex;
                Language newLang = EnumHelper.Languages[newIndex];

                bool found = false;
                foreach (var info in text.QuestInfo)
                {
                    if (info.Language == newLang.Identifier)
                    {
                        found = true;
                        QuestName.Text = info.Name;
                        QuestClient.Text = info.Client;
                        QuestDesc.Text = info.Description;
                        QuestTarget.Text = info.Target;
                        QuestFail.Text = info.Fail;

                        break;
                    }
                }

                if (!found)
                {
                    text.QuestInfo.Add(new QuestText.QuestInfo_(newLang.Identifier));

                    QuestName.Text = "";
                    QuestClient.Text = "";
                    QuestDesc.Text = "";
                    QuestTarget.Text = "";
                    QuestFail.Text = "";
                }

                _selectedLanguage = newIndex;
            }
        }

        private TextBox? GetSelectedTextBox()
        {
            if (QuestName.IsFocused)
                return QuestName;
            else if (QuestDesc.IsFocused)
                return QuestDesc;
            else if (QuestClient.IsFocused)
                return QuestClient;
            else if (QuestTarget.IsFocused)
                return QuestTarget;

            return null;
        }

        private void ApplyTextEditBold_Click(object sender, ExecutedRoutedEventArgs e)
        {
            if (_customQuest != null)
            {
                TextBox? textBox = GetSelectedTextBox();
                if (textBox != null)
                {
                    string text = textBox.Text;
                    int start = textBox.SelectionStart;
                    int end = start + textBox.SelectionLength;

                    textBox.Text = $"{text[..start]}<BOLD>{textBox.SelectedText}</BOLD>{text[end..]}";
                }
            }
        }

        private void ApplyTextEditItalic_Click(object sender, ExecutedRoutedEventArgs e)
        {
            if (_customQuest != null)
            {
                TextBox? textBox = GetSelectedTextBox();
                if (textBox != null)
                {
                    string text = textBox.Text;
                    int start = textBox.SelectionStart;
                    int end = start + textBox.SelectionLength;

                    textBox.Text = $"{text[..start]}<ITALIC>{textBox.SelectedText}</ITALIC>{text[end..]}";
                }
            }
        }

        private void ApplyTextEditShadow_Click(object sender, ExecutedRoutedEventArgs e)
        {
            if (_customQuest != null)
            {
                TextBox? textBox = GetSelectedTextBox();
                if (textBox != null)
                {
                    string text = textBox.Text;
                    int start = textBox.SelectionStart;
                    int end = start + textBox.SelectionLength;

                    textBox.Text = $"{text[..start]}<SHADOW>{textBox.SelectedText}</SHADOW>{text[end..]}";
                }
            }
        }

        private void ApplyTextEditGlow_Click(object sender, ExecutedRoutedEventArgs e)
        {
            if (_customQuest != null)
            {
                TextBox? textBox = GetSelectedTextBox();
                if (textBox != null)
                {
                    string text = textBox.Text;
                    int start = textBox.SelectionStart;
                    int end = start + textBox.SelectionLength;

                    textBox.Text = $"{text[..start]}<GLOW>{textBox.SelectedText}</GLOW>{text[end..]}";
                }
            }
        }

        private void ApplyTextEditSize_Click(object sender, ExecutedRoutedEventArgs e)
        {
            if (_customQuest != null)
            {
                TextBox? textBox = GetSelectedTextBox();
                if (textBox != null)
                {
                    string text = textBox.Text;
                    int start = textBox.SelectionStart;
                    int end = start + textBox.SelectionLength;

                    textBox.Text = $"{text[..start]}<SIZE 1>{textBox.SelectedText}</SIZE>{text[end..]}";
                }
            }
        }

        private void ApplyTextEditColor_Click(object sender, ExecutedRoutedEventArgs e)
        {
            if (_customQuest != null)
            {
                TextBox? textBox = GetSelectedTextBox();
                if (textBox != null)
                {

                    string text = textBox.Text;
                    int start = textBox.SelectionStart;
                    int end = start + textBox.SelectionLength;

                    textBox.Text = $"{text[..start]}<COLOR 000000>{textBox.SelectedText}</COLOR>{text[end..]}";
                }
            }
        }

        private void QuestInfo_SelectionChanged(object sender, RoutedEventArgs e)
        {
            bool colorDisplayed = false;

            if (_customQuest != null)
            {
                Regex regex = new(@"<(COLOR|color) ([0-9a-fA-F]+)>[^<]*</(COLOR|color)>");
                TextBox? textBox = GetSelectedTextBox();
                if (textBox != null)
                {
                    Match match = regex.Match(textBox.Text);
                    if (match.Success)
                    {
                        int start = match.Index;
                        int end = start + match.Length;
                        
                        if (textBox.SelectionStart >= start && textBox.SelectionStart + textBox.SelectionLength < end)
                        {
                            int color = int.Parse(match.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);

                            byte r = (byte)((color & 0xFF0000) >> 0x10);
                            byte g = (byte)((color & 0x00FF00) >> 0x08);
                            byte b = (byte)((color & 0x0000FF) >> 0x00);

                            ColorBox.Visibility = Visibility.Visible;
                            ColorBox.Fill = new SolidColorBrush(Color.FromRgb(r, g, b));
                            colorDisplayed = true;
                        }
                    }
                }
            }

            if (!colorDisplayed)
            {
                ColorBox.Visibility = Visibility.Hidden;
                ColorBox.Fill = Brushes.White;
            }
        }
    }

    public static class Command
    {
        public static readonly RoutedUICommand ApplyBold = new("Bold", "ApplyBold", typeof(MainWindow));
        public static readonly RoutedUICommand ApplyItalics = new("Italics", "ApplyItalics", typeof(MainWindow));
        public static readonly RoutedUICommand ApplyShadow = new("Shadow", "ApplyShadow", typeof(MainWindow));
        public static readonly RoutedUICommand ApplyGlow = new("Glow", "ApplyGlow", typeof(MainWindow));
        public static readonly RoutedUICommand ApplySize = new("Size", "ApplySize", typeof(MainWindow));
        public static readonly RoutedUICommand ApplyColor = new("Color", "ApplyColor", typeof(MainWindow));
    }
}
