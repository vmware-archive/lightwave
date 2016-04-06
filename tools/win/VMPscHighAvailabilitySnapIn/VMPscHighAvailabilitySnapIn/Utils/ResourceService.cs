﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using System.Drawing;
using System.IO;
using System.Reflection;

namespace VMPscHighAvailabilitySnapIn.Utils
{
    /// <summary>
    /// Resource helper
    /// </summary>
    public static class ResourceHelper
    {
        /// <summary>
        /// Gets the toolbar image
        /// </summary>
        /// <returns></returns>
        public static Image GetToolbarImage()
        {
            return GetResourceImage("VMPscHighAvailabilitySnapIn.Toolbar.bmp");
        }

        /// <summary>
        /// Gets a resource image
        /// </summary>
        /// <param name="name">Name of image</param>
        /// <returns>Image</returns>
        public static Image GetResourceImage(string name)
        {
            using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(name))
            {
                if (stream != null) return new Bitmap(stream);
            }
            var message = string.Format("Resource {0} not found", name);
            throw new Exception(message);
        }

        /// <summary>
        /// Gets resource icon
        /// </summary>
        /// <param name="name">Name of icon</param>
        /// <returns>Icon</returns>
        public static Icon GetResourceIcon(string name)
        {
            using (Stream stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(name))
            {
                if (stream != null) return new Icon(stream);
            }
            var message = string.Format("Resource {0} not found", name);
            throw new Exception(message);
        }
    }
}
