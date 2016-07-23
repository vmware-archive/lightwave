/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
using System;
using Foundation;
using AppKit;
using VMDir.Common.DTO;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.VMDirUtilities;
using VMDir.Common;
using System.Linq;
using VMDirSnapIn.DataSource;
using VMDirInterop.LDAP;
using System.Collections.Generic;
using VMDirSnapIn.Nodes;
using VMDirInterop.Interfaces;
using VMDirSnapIn.Delegate;
using VmIdentity.UI.Common;
using System.IO;
using System.Xml.Serialization;

namespace VMDirSnapIn.UI
{
	public partial class SearchWindowController : NSWindowController
	{
		private string _dn;
		private VMDirServerDTO _serverDTO;
		private SearchConditionsTableViewDataSource _searchCondDs;
		private PropertiesViewController _propViewController;
		private ResultOutlineDataSource _resultDs;
		private int _pageSize;
		private IntPtr _cookie = IntPtr.Zero;
		private int _totalCount = 0;
		private int _pageNumber = 1;
		private bool _morePages = false;
		private QueryDTO _qdto;
		private bool _searchBoxFlag = false;
		public List<DirectoryNode> _resultList { get; set; }
		private int _currPage { get; set; }
		private int _totalPage { get; set; }

		public SearchWindowController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public SearchWindowController(NSCoder coder) : base(coder)
		{
		}

		public SearchWindowController(string dn, VMDirServerDTO serverDTO) : base("SearchWindow")
		{
			this._dn = dn;
			this._serverDTO = serverDTO;
			_pageSize = 100;
			_currPage = 0;
			_totalPage = 0;
			_resultList = new List<DirectoryNode>();
		}

		void InitPageSearch()
		{
			_qdto = GetQuery();
			_cookie = IntPtr.Zero;
			_totalCount = 0;
			_pageNumber = 1;
			_morePages = false;
			_currPage = 0;
			_totalPage = 0;
			_resultList.Clear();
			_resultDs.ResultList.Clear();
			SetHeaderText("");
		}

		private void BindView()
		{
			this.BFSearchBaseTextField.StringValue = _dn;
			this.TFSearchBaseTextField.StringValue = _dn;

			foreach (var item in VMDirConstants.ScopeList)
			{
				this.BFSeachScopeComboBox.Add(new NSString(item));
				this.TFSearchScopeComboBox.Add(new NSString(item));
			}

			foreach (var item in VMDirConstants.ConditionList)
				this.BFCondComboBox.Add(new NSString(item));

			var attrTypes = _serverDTO.Connection.SchemaManager.GetAttributeTypeManager();
			var attrList = attrTypes.Data.Select(x => x.Key).ToList();
			attrList.Sort((x, y) => string.Compare(x, y, StringComparison.InvariantCultureIgnoreCase));
			foreach (var item in attrList)
			{
				this.BFAttrComboBox.Add(new NSString(item));
			}

			foreach (var item in VMDirConstants.OperatorList)
				this.BFOperatorComboBox.Add(new NSString(item));

			BFCondComboBox.SelectItem(0);
			BFSeachScopeComboBox.SelectItem(2);
			TFSearchScopeComboBox.SelectItem(2);
			BFAttrComboBox.SelectItem(0);
			BFOperatorComboBox.SelectItem(0);

		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			BindView();

			_propViewController = new PropertiesViewController();
			_propViewController.PropTableView = new VMDirTableView();
			_propViewController.View.SetFrameSize(ResultPropView.Frame.Size);
			ResultPropView.AddSubview(_propViewController.View);
			SearchBoxVisibilityToolBarItem.Active = true;
			//SearchResultContainerView.RemoveFromSuperview();
			RemoveTableColumns();

			//Populate appropriate columns
			NSTableColumn col = new NSTableColumn("Attribute");
			col.HeaderCell.Title = "Attribute";
			col.HeaderCell.Alignment = NSTextAlignment.Center;
			col.DataCell = new NSBrowserCell();
			col.MinWidth = 250;
			col.ResizingMask = NSTableColumnResizing.UserResizingMask;
			_propViewController.PropTableView.AddColumn(col);

			NSTableColumn col1 = new NSTableColumn("Value");
			col1.HeaderCell.Title = "Value";
			col1.ResizingMask = NSTableColumnResizing.UserResizingMask;
			col1.HeaderCell.Alignment = NSTextAlignment.Center;
			col1.MinWidth = 250;
			_propViewController.PropTableView.AddColumn(col1);

			NSTableColumn col2 = new NSTableColumn("Syntax");
			col2.HeaderCell.Title = "Syntax";
			col2.ResizingMask = NSTableColumnResizing.UserResizingMask;
			col2.HeaderCell.Alignment = NSTextAlignment.Center;
			col2.MinWidth = 200;
			_propViewController.PropTableView.AddColumn(col2);

			_searchCondDs = new SearchConditionsTableViewDataSource();
			BFCondTableView.DataSource = _searchCondDs;
			_resultDs = new ResultOutlineDataSource();
			SearchResultOutlineView.DataSource = _resultDs;
			SetHeaderText("");

			var col4 = SearchResultOutlineView.OutlineTableColumn;
			if (col4 != null)
				col4.DataCell = new NSBrowserCell();
			SearchResultOutlineView.Delegate = new ResultOutlineDelegate(this);

			ResultPageNoTextField.StringValue = _currPage.ToString();
			PageSizeToolBarItem.Active = true;
		}

		void SetHeaderText(string val)
		{
			SearchResultOutlineView.OutlineTableColumn.HeaderCell.Title = val;
		}

		partial void BFOnAddAction(NSObject sender)
		{
			if (!ValidateAdd())
				return;
			_searchCondDs.condList.Add(new FilterDTO(BFAttrComboBox.SelectedValue.ToString(), (Condition)(int)BFCondComboBox.SelectedIndex, BFValTextField.StringValue));
			BFCondTableView.ReloadData();
		}

		partial void BFOnSearchAction(NSObject sender)
		{
			DoSearch();
		}

		partial void TFOnSearchAction(NSObject sender)
		{
			DoSearch();
		}

		private void DoSearch()
		{
			InitPageSearch();
			if (!ValidateSearch())
				return;
			SetToolBarState(true);

			GetPage();

			_resultDs.ResultList.AddRange(_resultList.ToArray());
			SearchResultOutlineView.ReloadData();
			if (_resultList.Count > 0)
				_currPage = 1;
			else
				SetHeaderText(VMDirConstants.STAT_SR_NO_MATCH);
			ResultPageNoTextField.StringValue = _currPage.ToString();
		}

		private QueryDTO GetQuery()
		{
			QueryDTO qdto = null;
			if (SearchQueryTabView.IndexOf(SearchQueryTabView.Selected) == 0)
			{
				qdto = new BuildQueryDTO(BFSearchBaseTextField.StringValue, (LdapScope)(int)BFSeachScopeComboBox.SelectedIndex,
										 (LogicalOp)(int)BFOperatorComboBox.SelectedIndex,
										 _searchCondDs.condList, new string[] { VMDirConstants.ATTR_DN, VMDirConstants.ATTR_OBJECT_CLASS }, 0, IntPtr.Zero, 0);

			}
			else if (SearchQueryTabView.IndexOf(SearchQueryTabView.Selected) == 1)
			{
				qdto = new TextQueryDTO(TFSearchBaseTextField.StringValue, (LdapScope)(int)TFSearchScopeComboBox.SelectedIndex, TFSearchFilterTextView.Value,
					new string[] { VMDirConstants.ATTR_DN, VMDirConstants.ATTR_OBJECT_CLASS }, 0, IntPtr.Zero, 0);
			}
			return qdto;
		}

		private void GetPage()
		{
			UIErrorHelper.CheckedExec(delegate
			{
				_serverDTO.Connection.PagedSearch(_qdto, _pageSize, _cookie, _morePages,
					delegate (ILdapMessage ldMsg, IntPtr ck, bool moreP, List<ILdapEntry> entries)
					{
						_cookie = ck;
						_morePages = moreP;
						_totalCount += entries.Count();
						_pageNumber++;
						foreach (var entry in entries)
						{
							_resultList.Add(new DirectoryNode(entry.getDN(), Utilities.GetObjectClass(entry), _serverDTO, null));
						}
					});
				_totalPage = _totalCount / _pageSize;
				if (_totalCount % _pageSize > 0)
					_totalPage++;

				if (_morePages)
				{
					SetHeaderText(VMDirConstants.STAT_SR_MORE_PG);
				}
				else {
					SetHeaderText(VMDirConstants.STAT_SR_NO_MORE_PG);
				}
			});
		}

		partial void BFOnViewAction(NSObject sender)
		{
			var q = GetQuery();
			if (q != null)
				UIErrorHelper.ShowAlert(_qdto.GetFilterString(), "Query");
		}

		public new SearchWindow Window
		{
			get { return (SearchWindow)base.Window; }
		}

		private bool ValidateAdd()
		{
			if (BFAttrComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_ATTR);
				return false;

			}
			if (BFCondComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_COND);
				return false;
			}
			if (string.IsNullOrWhiteSpace(BFValTextField.StringValue))
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_VAL);
				return false;
			}
			return true;
		}

		private bool ValidateSearch()
		{
			if (string.IsNullOrWhiteSpace(this.BFSearchBaseTextField.StringValue))
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_SEARCH_BASE);
				return false;
			}
			if (BFSeachScopeComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_SEARCH_SCOPE);
				return false;
			}
			if (BFOperatorComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_OP);
				return false;
			}
			if (SearchQueryTabView.IndexOf(SearchQueryTabView.Selected) == 0 && BFCondTableView.RowCount <= 0)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_COND_COUNT);
				return false;
			}
			if (SearchQueryTabView.IndexOf(SearchQueryTabView.Selected) == 1 && string.IsNullOrWhiteSpace(TFSearchFilterTextView.Value))
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_TEXT_FILTER);
				return false;
			}
			return true;
		}

		public void RefreshPropTableViewBasedOnSelection(nint row)
		{
			if (row >= (nint)0)
			{
				NSObject item = SearchResultOutlineView.ItemAtRow(row);
				if (item is DirectoryNode)
				{
					DirectoryNode node = item as DirectoryNode;
					_propViewController.PropTableView.DataSource = new PropertiesTableViewDataSource(node.Dn, node.ObjectClass, node.ServerDTO, node.NodeProperties);
					_propViewController.ds = (PropertiesTableViewDataSource)_propViewController.PropTableView.DataSource;
					_propViewController.PropTableView.Delegate = new PropertiesTableDelegate(this, (PropertiesTableViewDataSource)_propViewController.PropTableView.DataSource);
				}
			}
			else
			{
				_propViewController.PropTableView.DataSource = null;
			}
			_propViewController.PropTableView.ReloadData();
		}

		private void RemoveTableColumns()
		{
			while (_propViewController.PropTableView.ColumnCount > 0)
			{
				_propViewController.PropTableView.RemoveColumn(_propViewController.PropTableView.TableColumns()[0]);
			}
		}

		partial void OnNextResultButton(NSObject sender)
		{
			_currPage++;
			if (_currPage > _totalPage && _morePages)
				GetPage();
			if (_currPage <= _totalPage && _currPage != 0)
			{
				_resultDs.ResultList.Clear();
				for (int i = (_currPage - 1) * _pageSize; i < _currPage * _pageSize && i < _resultList.Count; i++)
				{
					_resultDs.ResultList.Add(_resultList[i]);
				}
				ResultPageNoTextField.StringValue = _currPage.ToString();
				SearchResultOutlineView.ReloadData();
			}
			else
				_currPage--;
		}

		partial void OnPrevResultButton(NSObject sender)
		{
			_currPage--;
			if (_currPage >= 1)
			{
				_resultDs.ResultList.Clear();
				for (int i = (_currPage - 1) * _pageSize; i < _currPage * _pageSize && i < _resultList.Count; i++)
				{
					_resultDs.ResultList.Add(_resultList[i]);
				}
				ResultPageNoTextField.StringValue = _currPage.ToString();
				SearchResultOutlineView.ReloadData();
			}
			else
				_currPage++;
		}

		partial void OnLoadQueryToolBarItem(NSObject sender)
		{
			UIErrorHelper.CheckedExec(delegate
			{
				var open = NSOpenPanel.OpenPanel;
				open.AllowedFileTypes = new string[] { "xml" };
				open.Title = "Load Query";
				nint result = open.RunModal();
				if (result == (int)1)
				{
					string path = open.Url.Path;
					try
					{
						_qdto = LoadQuryOfType(typeof(BuildQueryDTO), path) as BuildQueryDTO;
					}
					catch (Exception)
					{
						_qdto = LoadQuryOfType(typeof(TextQueryDTO), path) as TextQueryDTO;
					}
					BindData();
					UIErrorHelper.ShowInformation(VMDirConstants.STAT_QUERY_LOAD_SUCC);
				}
			});
		}

		private object LoadQuryOfType(Type ty, string filename)
		{
			using (var ms = new MemoryStream())
			{
				var bytes = File.ReadAllBytes(filename);
				ms.Write(bytes, 0, bytes.Length);
				ms.Seek(0, SeekOrigin.Begin);

				var xmlSerializer = new XmlSerializer(ty);
				return xmlSerializer.Deserialize(ms);
			}
		}

		private void BindData()
		{
			if (_qdto.GetType() == typeof(BuildQueryDTO))
			{
				var dto = _qdto as BuildQueryDTO;
				SearchQueryTabView.SelectAt(0);
				BFSearchBaseTextField.StringValue = dto.SearchBase;
				BFSeachScopeComboBox.SelectItem((int)dto.SearchScope);
				BFOperatorComboBox.SelectItem((int)dto.Operator);
				BFAttrComboBox.SelectItem(0);
				BFCondComboBox.SelectItem(0);

				_searchCondDs.condList.Clear();
				foreach (var item in dto.CondList)
				{
					_searchCondDs.condList.Add(new FilterDTO(item.Attribute, item.Condition, item.Value));
				}
				BFCondTableView.ReloadData();
			}
			else if (_qdto.GetType() == typeof(TextQueryDTO))
			{
				var dto = _qdto as TextQueryDTO;
				SearchQueryTabView.SelectAt(1);
				TFSearchBaseTextField.StringValue = dto.SearchBase;
				TFSearchScopeComboBox.SelectItem((int)dto.SearchScope);
				TFSearchFilterTextView.Value = dto.GetFilterString();
			}
		}

		partial void OnStoreQueryToolBarItem(NSObject sender)
		{
			UIErrorHelper.CheckedExec(delegate ()
			{
				var data = GetQuery();
				if (data == null)
					return;

				using (var ms = new MemoryStream())
				{
					var xmlSerializer = new XmlSerializer(data.GetType());
					xmlSerializer.Serialize(ms, data);

					var save = NSSavePanel.SavePanel;
					save.AllowedFileTypes = new string[] { "xml" };
					save.Title = "Store Query";
					nint result = save.RunModal();
					if (result == (int)1)
					{
						string path = save.Url.Path;
						File.WriteAllBytes(path, ms.ToArray());
						UIErrorHelper.ShowInformation(VMDirConstants.STAT_QUERY_STORE_SUCC);
					}
				}
			});
		}

		partial void OnPageSizeToolBarItem(NSObject sender)
		{
			PageSizeController pswc = new PageSizeController(_pageSize);
			NSApplication.SharedApplication.BeginSheet(pswc.Window, this.Window, () =>
				{
				});
			try
			{
				nint result = NSApplication.SharedApplication.RunModalForWindow(pswc.Window);
				if (result == (nint)VMIdentityConstants.DIALOGOK)
				{
					_pageSize = pswc.PageSize;
					_totalPage = _totalCount / _pageSize;
					if (_totalCount % _pageSize > 0)
						_totalPage++;
				}
			}
			finally
			{
				Window.EndSheet(pswc.Window);
				pswc.Dispose();
			}
		}

		partial void OnOperationalToolBarItem(NSObject sender)
		{
			if (_serverDTO.OperationalAttrFlag)
				_serverDTO.OperationalAttrFlag = false;
			else
				_serverDTO.OperationalAttrFlag = true;
			RefreshPropTableViewBasedOnSelection(SearchResultOutlineView.SelectedRow);
		}

		partial void OnSearchBoxVisibilityToolBarItem(Foundation.NSObject sender)
		{
			SetSearchBoxVisibility();
		}

		private void SetSearchBoxVisibility()
		{
			if (_searchBoxFlag)
			{
				SearchQueryContainerView.RemoveFromSuperview();	
				//SearchHorizontalSplitView.RemoveArrangedSubview(SearchQueryContainerView);
				_searchBoxFlag = false;
			}
			else {
				foreach (var item in SearchHorizontalSplitView.Subviews)
					//SearchHorizontalSplitView.RemoveArrangedSubview(item);//supported 10.11 onward
					item.RemoveFromSuperview();

				var size = SearchHorizontalSplitView.Frame.Size;
				SearchQueryContainerView.SetFrameSize(new CoreGraphics.CGSize(size.Width, 280));
				//SearchHorizontalSplitView.AddArrangedSubview(SearchQueryContainerView);
				SearchHorizontalSplitView.AddSubview(SearchQueryContainerView);
				SearchResultContainerView.SetFrameSize(new CoreGraphics.CGSize(size.Width, size.Height - 280));
				//SearchHorizontalSplitView.AddArrangedSubview(SearchResultContainerView);
				SearchHorizontalSplitView.AddSubview(SearchResultContainerView);
				_searchBoxFlag = true;
			}
		}

		private void SetToolBarState(bool state)
		{
			OperationalAttrToolBarItem.Active = state;
			PageSizeToolBarItem.Active = state;
			StoreQueryToolBarItem.Active = state;
			LoadQueryToolBarItem.Active = state;
			SearchBoxVisibilityToolBarItem.Active = state;
		}

		partial void OnRemoveTableEntry(Foundation.NSObject sender)
		{
			nint row = BFCondTableView.SelectedRow;
			if (row >= (nint)0)
			{
				_searchCondDs.condList.RemoveAt((int)row);
				BFCondTableView.ReloadData();
			}
		}
	}
}
