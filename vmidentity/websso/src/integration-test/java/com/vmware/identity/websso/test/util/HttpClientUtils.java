/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

package com.vmware.identity.websso.test.util;

import com.vmware.identity.websso.test.util.common.Assert;
import com.vmware.identity.websso.test.util.common.SAMLConstants;

import org.apache.commons.codec.binary.Base64;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.InetAddress;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.LogManager;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.commons.io.IOUtils;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class HttpClientUtils {
  private static final Logger log = LoggerFactory.getLogger(HttpClientUtils.class);

  private static final String UTF8_CHARSET = "UTF-8";
  // Node of 'input' with 'name' attribute
  private static final String INPUT_NAME_ATTR = "input[name]";
  private static final String VALUE_ATTR = "value";

  public static void disableInfoLevel(String loggerName) {
    // If you want to turn on debug logging and it is enabled or want to
    // turn it off
    //and it is not enabled do nothing
    if (LogManager.getLogManager() != null && LogManager.getLogManager().getLogger(loggerName) != null) {
      LogManager.getLogManager().getLogger(loggerName).setLevel(Level.WARNING);
    }
  }

  public static String getResponseContent(HttpResponse response) throws IllegalStateException, IOException {
    HttpEntity entity = response.getEntity();
    // entity.consumeContent();

    if (entity == null) {
      return null;
    }

    String encoding = (entity.getContentEncoding() == null) ? null : entity.getContentEncoding().getValue();

    if (entity.isChunked()) {
      StringWriter writer1 = new StringWriter();
      if (encoding == null) {
        IOUtils.copy(entity.getContent(), writer1);
      } else {
        IOUtils.copy(entity.getContent(), writer1, entity.getContentEncoding().getValue());
      }
      return writer1.toString();
    } else {
      StringWriter writer2 = new StringWriter();
      if (encoding == null) {
        IOUtils.copy(entity.getContent(), writer2);
      } else {
        IOUtils.copy(entity.getContent(), writer2, entity.getContentEncoding().getValue());
      }
      return writer2.toString();
    }
  }

  // Utility function to convert to Base64
  public static String ConvertToBase64(String rawStr) {
    byte[] bytes = Base64.encodeBase64(rawStr.getBytes());
    return new String(bytes);
  }

  public static byte[] convertToBase64(String rawStr) {
    return Base64.encodeBase64(rawStr.getBytes());
  }

  // Utility function to convert to Base64 using byte array
  public static String convertToBase64(byte[] bytes) {
    byte[] base64Bytes = Base64.encodeBase64(bytes);
    return new String(base64Bytes);
  }

  // Utility to parse html data
  public static String getEncodedSamlToken(String htmlResponse) {
    // Parse the whole html document
    Document document = Jsoup.parse(htmlResponse, UTF8_CHARSET);
    Document parse = Jsoup.parse(document.html());
    // Get the first 'input' node with 'name' attr. Note: There's only one
    Element samlResp = parse.select(INPUT_NAME_ATTR).first();
    Assert.assertNotNull(samlResp, "Cannot find the 'input' node in the html response");
    // Get the 'value' attr in the node.
    String encodedSaml = samlResp.attr(VALUE_ATTR);
    Assert.assertFalse(encodedSaml.length() == 0, "Cannot find the 'SamlResponse' in the html response");
    return encodedSaml;
  }

  public static String getRedirectURL(String htmlResponse) {
    // Parse the whole html document
    Document document = Jsoup.parse(htmlResponse, UTF8_CHARSET);
    Document parse = Jsoup.parse(document.html());
    Element formElement = parse.select("form").first();
    Assert.assertNotNull(formElement, "Cannot find the 'form' node in the html response");
    return formElement.attr("action");
  }

  public static String base64Decode(String encodedDataStr) {
    // Decode the encoded response
    byte[] decodedData = Base64.decodeBase64(encodedDataStr);
    String decodedDataStr = new String(decodedData);
    return decodedDataStr;
  }

  public static String excStackToString(Exception ex) {
    StringWriter errors = new StringWriter(); ex.printStackTrace(new PrintWriter(errors));
    String errorStack = errors.toString(); try {
      errors.close();
    } catch (IOException e) {
      log.error(e.toString());
    } finally {
      return errorStack;
    }
  }

  public static void logException(Logger log, Exception exc) {
    logException(log, exc, "");
  }

  public static void logException(Logger log, Exception exc, String message) {
    String logString = String.format(
                          "%s\nException messaage: %s\nException stack: %s\n",
                          message, exc.getMessage(),
                          excStackToString(exc)
                        );
    log.error(logString);
  }
}
