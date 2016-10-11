/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
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
using System.Runtime.InteropServices;

namespace VMDirSnapIn.UI
{
	public partial class SearchWindowController : NSWindowController
	{
		private string _dn;
		private VMDirServerDTO _serverDTO;
		private SearchConditionsTableViewDataSource _searchCondDs;
		private AttributeTableViewDataSource _attrToReturnDs;
		private PropertiesViewController _propViewController;
		private ResultOutlineDataSource _resultDs;
		private int _pageSize;
		private IntPtr _cookie = IntPtr.Zero;
		private int _totalCount = 0;
		private int _pageNumber = 1;
		private bool _morePages = false;
		private QueryDTO _qdto;
		private bool _searchBoxFlag = false;
		public List<DirectoryNonExpandableNode> _resultList { get; set; }
		private int _currPage { get; set; }
		private int _totalPage { get; set; }
		private List<string> _attrList;
		private List<string> _returnedAttrList;

		private NSObject ReloadResultOutlineViewNotificationObject;
		private NSObject ReloadResultTableViewNotificationObject;
		private NSObject CloseSearchNotificationObject;

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
			_resultList = new List<DirectoryNonExpandableNode>();
			_returnedAttrList = new List<string>();
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
			_returnedAttrList.Clear();
			_returnedAttrList.AddRange(_qdto.AttrToReturn);
			SetHeaderText("");
		}

		private void BindView()
		{
			this.SearchBaseTextField.StringValue = _dn;
			foreach (var item in VMDirConstants.ScopeList)
			{
				this.SearchScopeComboBox.Add(new NSString(item));
			}

			foreach (var item in VMDirConstants.ConditionList)
				this.BfConditionComboBox.Add(new NSString(item));

			var attrTypes = _serverDTO.Connection.SchemaManager.GetAttributeTypeManager();
			_attrList = attrTypes.Data.Select(x => x.Key).ToList();
			_attrList.Sort((x, y) => string.Compare(x, y, StringComparison.InvariantCultureIgnoreCase));
			foreach (var item in _attrList)
			{
				this.BfAttributeComboBox.Add(new NSString(item));
				this.AttrToReturnComboBox.Add(new NSString(item));
			}

			foreach (var item in VMDirConstants.OperatorList)
				this.BfOperatorComboBox.Add(new NSString(item));

			BfConditionComboBox.SelectItem(0);
			SearchScopeComboBox.SelectItem(2);
			BfAttributeComboBox.SelectItem(0);
			BfOperatorComboBox.SelectItem(0);
			AttrToReturnComboBox.SelectItem(0);

		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			BindView();
			ReloadResultOutlineViewNotificationObject = NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadResultOutlineView", ReloadResultOutlineView);
			ReloadResultTableViewNotificationObject = NSNotificationCenter.DefaultCenter.AddObserver((NSString)"ReloadResultTableView", ReloadResultTableView);
			CloseSearchNotificationObject = NSNotificationCenter.DefaultCenter.AddObserver((NSString)"CloseSearchApplication", OnCloseSearchApplication);

			_propViewController = new PropertiesViewController();
			_propViewController.PropTableView = new VMDirTableView();
			_propViewController.View.SetFrameSize(ResultPropView.Frame.Size);
			ResultPropView.AddSubview(_propViewController.View);
			LoadQueryToolBarItem.Active = true;
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
			BfConditionsTableView.DataSource = _searchCondDs;
			_attrToReturnDs = new AttributeTableViewDataSource();
			AttrToReturnTableView.DataSource = _attrToReturnDs;
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

		void OnCloseSearchApplication(NSNotification obj)
		{
			NSNotificationCenter.DefaultCenter.RemoveObserver(ReloadResultOutlineViewNotificationObject);
			NSNotificationCenter.DefaultCenter.RemoveObserver(ReloadResultTableViewNotificationObject);
			NSNotificationCenter.DefaultCenter.RemoveObserver(CloseSearchNotificationObject);
		}

		void ReloadResultTableView(NSNotification obj)
		{
			RefreshPropTableViewBasedOnSelection(SearchResultOutlineView.SelectedRow);
		}

		void ReloadResultOutlineView(NSNotification obj)
		{
			var node = obj.Object as DirectoryNonExpandableNode;
			if (node != null)
			{
				_resultList.Remove(node);
				_resultDs.ResultList.Remove(node);
				SearchResultOutlineView.ReloadData();
			}
		}

		void SetHeaderText(string val)
		{
			SearchResultOutlineView.OutlineTableColumn.HeaderCell.Title = val;
		}

		partial void OnBfAddAction(NSObject sender)
		{
			if (!ValidateAdd())
				return;
			_searchCondDs.condList.Add(new FilterDTO(BfAttributeComboBox.SelectedValue.ToString(), (Condition)(int)BfConditionComboBox.SelectedIndex, BfValueTextField.StringValue));
			BfConditionsTableView.ReloadData();
		}

		partial void OnSearchAction(NSObject sender)
		{
			InitPageSearch();
			if (!ValidateSearch())
				return;
			SetToolBarState(true);
			_resultDs.ResultList.Clear();
			SearchResultOutlineView.ReloadData();
			RefreshPropTableViewBasedOnSelection(-1);
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
			var lst = new HashSet<string>(_attrToReturnDs.attrList);
			lst.Add(VMDirConstants.ATTR_OBJECT_CLASS);
			lst.Add(VMDirConstants.ATTR_DN);
			if (SearchQueryTabView.IndexOf(SearchQueryTabView.Selected) == 0)
			{
				qdto = new BuildQueryDTO(SearchBaseTextField.StringValue, (LdapScope)(int)SearchScopeComboBox.SelectedIndex,
										 (LogicalOp)(int)BfOperatorComboBox.SelectedIndex,
				                         _searchCondDs.condList, lst.ToArray(), 0, IntPtr.Zero, 0);

			}
			else if (SearchQueryTabView.IndexOf(SearchQueryTabView.Selected) == 1)
			{
				qdto = new TextQueryDTO(SearchBaseTextField.StringValue, (LdapScope)(int)SearchScopeComboBox.SelectedIndex, TfSearchFilterTextView.Value,
				                        lst.ToArray(), 0, IntPtr.Zero, 0);
			}
			return qdto;
		}

		private void GetPage()
		{
			SetHeaderText(VMDirConstants.STAT_SR_FETCHING_PG);
			//_qdto.TimeOut = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(Int32)));
			//Marshal.WriteInt32(_qdto.TimeOut, VMDirConstants.SEARCH_TIMEOUT_IN_SEC);
			try
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
							var ocList = new List<string>(entry.getAttributeValues(VMDirConstants.ATTR_OBJECT_CLASS).Select(x => x.StringValue).ToArray());
							var node = new DirectoryNonExpandableNode(entry.getDN(), ocList, _serverDTO);
							node.NodeProperties = _serverDTO.Connection.GetEntryProperties(entry);
							_resultList.Add(node);
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
			}
			catch (Exception e)
			{
				UIErrorHelper.ShowError(e.Message);
			}
			finally
			{
				//Marshal.FreeHGlobal(_qdto.TimeOut);
			}
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
			if (BfAttributeComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_ATTR);
				return false;

			}
			if (BfConditionComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_COND);
				return false;
			}
			if (string.IsNullOrWhiteSpace(BfValueTextField.StringValue))
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_VAL);
				return false;
			}
			return true;
		}

		private bool ValidateSearch()
		{
			if (string.IsNullOrWhiteSpace(this.SearchBaseTextField.StringValue))
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_SEARCH_BASE);
				return false;
			}
			if (SearchScopeComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_SEARCH_SCOPE);
				return false;
			}
			if (BfOperatorComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_OP);
				return false;
			}
			if (SearchQueryTabView.IndexOf(SearchQueryTabView.Selected) == 0 && BfConditionsTableView.RowCount <= 0)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_COND_COUNT);
				return false;
			}
			if (SearchQueryTabView.IndexOf(SearchQueryTabView.Selected) == 1 && string.IsNullOrWhiteSpace(TfSearchFilterTextView.Value))
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
					_propViewController.PropTableView.DataSource = new PropertiesTableViewDataSource(node.Dn, node.ObjectClass.Last(), node.ServerDTO, node.NodeProperties);
					_propViewController.ds = (PropertiesTableViewDataSource)_propViewController.PropTableView.DataSource;
					_propViewController.PropTableView.Delegate = new PropertiesTableDelegate(this, (PropertiesTableViewDataSource)_propViewController.PropTableView.DataSource,_propViewController);
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
				SearchBaseTextField.StringValue = dto.SearchBase;
				SearchScopeComboBox.SelectItem((int)dto.SearchScope);
				BfOperatorComboBox.SelectItem((int)dto.Operator);
				BfAttributeComboBox.SelectItem(0);
				BfConditionComboBox.SelectItem(0);

				_searchCondDs.condList.Clear();
				foreach (var item in dto.CondList)
				{
					_searchCondDs.condList.Add(new FilterDTO(item.Attribute, item.Condition, item.Value));
				}
				BfConditionsTableView.ReloadData();
				_attrToReturnDs.attrList.Clear();
				foreach (var item in dto.AttrToReturn)
				{
					_attrToReturnDs.attrList.Add(item);
				}
				AttrToReturnTableView.ReloadData();
			}
			else if (_qdto.GetType() == typeof(TextQueryDTO))
			{
				var dto = _qdto as TextQueryDTO;
				SearchQueryTabView.SelectAt(1);
				SearchBaseTextField.StringValue = dto.SearchBase;
				SearchScopeComboBox.SelectItem((int)dto.SearchScope);
				TfSearchFilterTextView.Value = dto.GetFilterString();
				_attrToReturnDs.attrList.Clear();
				foreach (var item in dto.AttrToReturn)
				{
					_attrToReturnDs.attrList.Add(item);
				}
				AttrToReturnTableView.ReloadData();
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

		partial void OnOptionalToolBarItem(NSObject sender)
		{
			if (_serverDTO.OptionalAttrFlag)
				_serverDTO.OptionalAttrFlag = false;
			else
				_serverDTO.OptionalAttrFlag = true;
			RefreshPropTableViewBasedOnSelection(SearchResultOutlineView.SelectedRow);
		}

		partial void OnDelete(NSObject sender)
		{
			nint row = SearchResultOutlineView.SelectedRow;
			if (isObjectSelected(row))
			{
				DirectoryNonExpandableNode node = SearchResultOutlineView.ItemAtRow(row) as DirectoryNonExpandableNode;
				node.PerformDelete();
			}
		}

		partial void OnRefresh(NSObject sender)
		{
			nint row = SearchResultOutlineView.SelectedRow;
			if (isObjectSelected(row))
			{
				DirectoryNonExpandableNode node = SearchResultOutlineView.ItemAtRow(row) as DirectoryNonExpandableNode;
				node.PerformRefreshNode();
			}
		}

		partial void OnSearchBoxVisibilityToolBarItem(Foundation.NSObject sender)
		{
			SetSearchBoxVisibility();
		}

		private bool isObjectSelected(nint row)
		{
			if (row < (nint)0)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_OBJ_NODE_SEL);
				return false;
			}
			else
				return true;
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
			OptionalToolBarItem.Active = state;
			PageSizeToolBarItem.Active = state;
			StoreQueryToolBarItem.Active = state;
			LoadQueryToolBarItem.Active = state;
			SearchBoxVisibilityToolBarItem.Active = state;
			RefreshToolBarItem.Active = state;
			DeleteToolBarItem.Active = state;
			ExportToolBarItem.Active = state;
		}

		partial void OnBfRemoveTableEntry(Foundation.NSObject sender)
		{
			nint row = BfConditionsTableView.SelectedRow;
			if (row >= (nint)0)
			{
				_searchCondDs.condList.RemoveAt((int)row);
				BfConditionsTableView.ReloadData();
			}
		}
		partial void OnBfRemoveAllTableEntries(NSObject sender)
		{
			_searchCondDs.condList.Clear();
			BfConditionsTableView.ReloadData();
		}
		partial void OnBfCopyToTf(NSObject sender)
		{
			var query = GetQuery();
			if (query != null)
			{
				this.TfSearchFilterTextView.Value = query.GetFilterString();
				this.SearchQueryTabView.SelectAt(1);
			}
		}
		partial void OnBfMultipleValFromFile(NSObject sender)
		{
			ConditionValuesFromFileController cvffwc = new ConditionValuesFromFileController(_attrList);
			nint result = NSApplication.SharedApplication.RunModalForWindow(cvffwc.Window);
			if (result == (nint)VMIdentityConstants.DIALOGOK)
			{
				foreach (var item in cvffwc.ValuesList)
				{
					_searchCondDs.condList.Add(new FilterDTO(cvffwc.Attribute,cvffwc.Condition,item));
				}
				BfConditionsTableView.ReloadData();
			}
		}
		partial void OnAttrToReturnAdd(NSObject sender)
		{
			_attrToReturnDs.attrList.Add(AttrToReturnComboBox.SelectedValue.ToString());
			AttrToReturnTableView.ReloadData();
		}
		partial void OnAttrToReturnRemove(NSObject sender)
		{
			nint row = AttrToReturnTableView.SelectedRow;
			if (row >= (nint)0)
			{
				_attrToReturnDs.attrList.RemoveAt((int)row);
				AttrToReturnTableView.ReloadData();
			}
		}
		partial void OnAttrToReturnRemoveAll(NSObject sender)
		{
			_attrToReturnDs.attrList.Clear();
			AttrToReturnTableView.ReloadData();
		}
		partial void OnExportToolBarItem(NSObject sender)
		{
			ExportSearchResultController esrwc = new ExportSearchResultController(_resultList,_returnedAttrList,_currPage,_pageSize);
			NSApplication.SharedApplication.RunModalForWindow(esrwc.Window);
		}

		[Export("windowWillClose:")]
		public void WindowWillClose(NSNotification notification)
		{
			NSApplication.SharedApplication.StopModalWithCode(0);
			NSNotificationCenter.DefaultCenter.PostNotificationName("CloseSearchApplication", this);
		}
	}
}
