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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace VMDirSnapIn.Utilities
{
    public class SearchListByKeyPress
    {
        public static void findAndSelect(ListView listView, char key)
        {
            char keyPressed = Char.ToLower(key);

            if (keyPressed < 'a' || keyPressed > 'z')                                //key press should be an alphabhat.
            {
                return;
            }
            if (listView.Items.Count == 0)                                           //special case when there is no item in the ListView.
            {
                return;
            }
            if (listView.Items.Count == 1)                                           //special case when there is only one item in the ListView.
            {
                if (listView.Items[0].Text[0] == keyPressed)
                {
                    listView.Items[0].Selected = true;
                }
                else
                {
                    return;
                }
            }

            var currentIndex = -1;
            if (listView.SelectedIndices.Count != 0)
            {
                currentIndex = listView.SelectedIndices[0];
            }

            var searchIndex = searchIndexByChar(listView, keyPressed, currentIndex); //search the next element which start with the char key pressed.

            if (searchIndex != -1 && currentIndex != searchIndex)
            {
                if (currentIndex != -1)
                {
                    listView.Items[currentIndex].Selected = false;
                }
                listView.Items[searchIndex].Selected = true;
                listView.EnsureVisible(searchIndex);
            }

        }

        private static int searchIndexByChar(ListView listView, char keyPressed, int currentIndex)
        {
            int count = listView.Items.Count;
            int searchIndex = currentIndex + 1;

            if (searchIndex < count && listView.Items[searchIndex].Text[0] <= keyPressed)
            {
                while (searchIndex < count)
                {
                    if (listView.Items[searchIndex].Text[0] == keyPressed)
                        return searchIndex;
                    else if (listView.Items[searchIndex].Text[0] > keyPressed)
                        break;
                    searchIndex++;
                }
            }
            else
            {
                searchIndex = currentIndex;
                while (searchIndex >= 0)
                {
                    if (listView.Items[searchIndex].Text[0] == keyPressed && ( (searchIndex-1) < 0 || listView.Items[searchIndex - 1].Text[0] != keyPressed))
                        return searchIndex;
                    else if (listView.Items[searchIndex].Text[0] < keyPressed)
                        break;
                    searchIndex--;
                }
            }
            return -1;
        }
    }
}
