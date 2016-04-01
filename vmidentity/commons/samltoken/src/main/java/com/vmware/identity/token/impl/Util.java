/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.token.impl;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import javax.xml.XMLConstants;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Node;
import org.w3c.dom.ls.LSInput;
import org.w3c.dom.ls.LSResourceResolver;
import org.xml.sax.SAXException;

import com.vmware.identity.token.impl.exception.ParserException;
import com.vmware.vim.sso.client.SamlToken;

/**
 * Various utility methods.
 */
public final class Util {

   private static Logger getLog() {
      return LoggerFactory.getLogger(Util.class);
   }

   /**
    * Serializes DOM Node to its String representation.
    *
    * <p>
    * The XML declaration <?xml ... > is omitted for easier
    * embedding/serialization.
    *
    * @param content, required
    * @return XML as string without XML declaration
    * @throws TransformerException
    */
   public static String serializeToString(Node content) throws ParserException {
      TransformerFactory tf = TransformerFactory.newInstance();
      StringWriter writer = new StringWriter();
      try {
         Transformer trans = tf.newTransformer();
         trans.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
         // no need to set encoding here
         trans.transform(new DOMSource(content), new StreamResult(writer));
      } catch (TransformerException e) {
         String message = "Error while serializing Node to String";
         getLog().error(message);
         throw new ParserException(message, e);
      }

      return writer.toString();
   }

   /**
    * Creates a description of the token without any private information
    *
    * @param token
    *           required, must be validated
    * @return description, not null TODO migrate this code into samltoken
    *         implementation
    */
   public static String createRedactedDescription(SamlToken token) {
      // relies on TokenDelegate objects in chain, having toString()
      return String.format(
         "%s [subject=%s, groups=%s, delegactionChain=%s, startTime=%s, "
            + "expirationTime=%s, renewable=%s, delegable=%s, isSolution=%s,"
            + "confirmationType=%s]", token.getClass().getSimpleName(),
         getTokenSubjectForLog(token), token.getGroupList(), token.getDelegationChain(),
         token.getStartTime(), token.getExpirationTime(), token.isRenewable(),
         token.isDelegable(), token.isSolution(), token.getConfirmationType());
   }

   static String getTokenSubjectForLog(SamlToken token){
       String tokenSubject = null;
       if(token != null){
           if (token.getSubject() != null){
               tokenSubject = token.getSubject().toString();
           } else if ( token.getSubjectNameId() != null ) {
               tokenSubject = token.getSubjectNameId().toString();
           }
       }
       if( tokenSubject == null ){
           tokenSubject = "NULL";
       }
       return tokenSubject;
   }

   /**
    * Supports internal com.vmware.identity infrastructure. Not to be consumed by external clients.
    */
   public static Schema loadXmlSchemaFromResource(ClassLoader theClassLoader, String resourceLocation, String resourceName)
           throws IllegalArgumentException, SAXException
   {
       // This function will try to load xml schema which is stored in the webapp's resources.
       // being able to be loaded by specified theClassLoader.
       // The schema file is allowed to reference other schema files, which must
       // reside in the same location.
       try
       {
           getLog().debug("loadXmlSchemaFromResource([{}], [{}], [{}])", new Object[] {theClassLoader.getClass().getName(), resourceLocation, resourceName});

           IResourceProvider resourceProvider = new ClassLoaderResourceProvider(theClassLoader, resourceLocation);
           try
           {
               return loadXmlSchemaFromResource(resourceProvider, resourceName);
           }
           finally
           {
               resourceProvider.close();
           }
       }
       catch (IOException e)
       {
           throw new IllegalArgumentException(e.getMessage(), e);
       }
   }

   /**
    * Supports internal com.vmware.identity infrastructure. Not to be consumed by external clients.
    */
   public static Schema loadXmlSchemaFromResource(Class<?> theClass, String resourceName)
           throws IllegalArgumentException, SAXException
   {
       // This function will try to load xml schema which is stored in file(s) packaged in the same jar file
       // as a specified theClass class; The schema file is allowed to reference other schema files, which must
       // reside in the same jar file as resources.
       //
       // bugzilla#988041, 1032736;
       try
       {
           getLog().debug("loadXmlSchemaFromResource([{}], [{}])", theClass.getName(), resourceName);
           IResourceProvider resourceProvider = new ClassResourceProvider(theClass, resourceName);
           try
           {
               return loadXmlSchemaFromResource(resourceProvider, resourceName);
           }
           finally
           {
               resourceProvider.close();
           }
       }
       catch (IOException e)
       {
           throw new IllegalArgumentException(e.getMessage(), e);
       }
   }

   private Util() {
      // prevent instantiation
   }

   private static Schema loadXmlSchemaFromResource(IResourceProvider resourceProvider, String resourceName)
           throws IllegalArgumentException, SAXException
   {
       ResourceResolver resolver = new ResourceResolver(resourceProvider);
       // load a schema, represented by a Schema instance
       InputStream is = resourceProvider.getResource(resourceName);
       if (is == null)
       {
           String message = String.format(
                   "Schema resource `%s' is missing.",
                   resourceName);
           IllegalArgumentException e = new IllegalArgumentException(message);
           getLog().error( message, e );
           throw e;
       }
       getLog().debug("Resources's [{}] InputStream type is: [{};{}]", new Object[] {resourceName, is.getClass().getName(), is.toString()});

       SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
       sf.setResourceResolver(resolver);

       Source schemaSource = new StreamSource(is);

       Schema schema = sf.newSchema(schemaSource);
       return schema;
   }

   private static interface IResourceProvider extends Closeable
   {
       public InputStream getResource(String resourceName);
   }
   private static class ClassLoaderResourceProvider implements IResourceProvider
   {
       private final String _resourceLocation;
       private final ClassLoader _classLoader;
       private List<InputStream> _streams;

       public ClassLoaderResourceProvider(ClassLoader theClassLoader, String resourceLocation)
       {
           this._classLoader = theClassLoader;
           this._streams = new ArrayList<InputStream>(10);
           this._resourceLocation =
               (((resourceLocation == null)||(resourceLocation.isEmpty()))
                ? ""
                : ((resourceLocation.endsWith("/")
                   ? resourceLocation :
                   resourceLocation + "/"))
               );
       }

       @Override
       public InputStream getResource(String resourceName)
       {
           InputStream is = this._classLoader.getResourceAsStream(this._resourceLocation + resourceName);
           if( is != null )
           {
               this._streams.add(is);
           }
           return is;
       }

       @Override
       public void close() throws IOException
       {
           if( this._streams != null )
           {
               for(InputStream is : this._streams)
               {
                   try
                   {
                       is.close();
                   }
                   catch(IOException ex)
                   {
                       Util.getLog().error("Closing resource stream had thrown exception. Ignoring.", ex);
                   }
               }
               this._streams = null;
           }
       }
   }

   private static class ClassResourceProvider implements IResourceProvider
   {
       private final Class<?> _class;
       private ZipFile _zipFile;
       private final String _resourcePath;
       private List<InputStream> _streams;
       private final static String FILE_SCHEME = "file";
       private final static String FILE_SCHEME_PREFIX = FILE_SCHEME + ":";

       public ClassResourceProvider(Class<?> theClass, String mainResourceName) throws IOException
       {
           this._class = theClass;
           this._resourcePath = this._class.getPackage().getName().replace('.', '/') + "/";
           this._streams = new ArrayList<InputStream>(10);
           URL url = this._class.getResource(mainResourceName);
           if(url == null)
           {
               Util.getLog().error("Unable to find resource [{}].", mainResourceName);
               throw new IllegalArgumentException("unable to locate resource " + mainResourceName);
           }
           if ( ( "jar".equalsIgnoreCase(url.getProtocol()) ) &&
                ( url.getPath() != null ) &&
                ( url.getPath().toLowerCase().startsWith( FILE_SCHEME_PREFIX ) )
              )
           {
               String filePath = url.getPath().substring(FILE_SCHEME_PREFIX.length());
               int index = filePath.lastIndexOf('!');
               if( index > -1 )
               {
                   filePath = filePath.substring(0, index);
               }

               if (filePath.isEmpty())
               {
                   throw new IllegalArgumentException("unable to locate resource due to filePath being empty");
               }

               Util.getLog().info("Reading resources from zip file path=[{}] ", filePath);
               filePath = decode(filePath, "UTF-8");
               Util.getLog().info("Reading resources from decoded zip file path=[{}] ", filePath);

               File file = new File(filePath);
               this._zipFile = new ZipFile(file, ZipFile.OPEN_READ);

           }
           else
           {
               this._zipFile = null;
           }
      }

       /**
        * Replacement for {@link java.net.URLDecoder#decode(String, String)}<code>.
        * <p>
        * <em><strong>Note:</strong> Does <strong>NOT</strong>
        * replace '<code>+</code>' (plus sign) with '<code> </code>' (space). </em>
        * <p>
        * Decodes a <code>application/x-www-form-urlencoded</code> string using a specific
        * encoding scheme.
        * The supplied encoding is used to determine
        * what characters are represented by any consecutive sequences of the
        * form "<code>%<i>xy</i></code>".
        * <p>
        * <em><strong>Note:</strong> The <a href=
        * "http://www.w3.org/TR/html40/appendix/notes.html#non-ascii-chars">
        * World Wide Web Consortium Recommendation</a> states that
        * UTF-8 should be used. Not doing so may introduce
        * incompatibilites.</em>
        *
        * @param s the <code>String</code> to decode
        * @param enc   The name of a supported character encoding.
        * @return the newly decoded <code>String</code>
        * @exception  UnsupportedEncodingException
        *             If character encoding needs to be consulted, but
        *             named character encoding is not supported
        * @see URLEncoder#encode(java.lang.String, java.lang.String)
        */
       private static String decode(String s, String enc) throws UnsupportedEncodingException {
          boolean needToChange = false;
          int numChars = s.length();
          StringBuffer sb = new StringBuffer(numChars > 500 ? numChars / 2 : numChars);
          int i = 0;

          if (enc.length() == 0) {
             throw new UnsupportedEncodingException(
                   "Decode: empty string enc parameter");
          }

          char c;
          byte[] bytes = null;
          while (i < numChars) {
             c = s.charAt(i);
             if (c == '%') {
                /*
                 * Starting with this instance of %, process all
                 * consecutive substrings of the form %xy. Each
                 * substring %xy will yield a byte. Convert all
                 * consecutive  bytes obtained this way to whatever
                 * character(s) they represent in the provided
                 * encoding.
                 */

                try {
                   // (numChars-i)/3 is an upper bound for the number
                   // of remaining bytes
                   if (bytes == null) {
                      bytes = new byte[(numChars-i)/3];
                   }
                   int pos = 0;

                   while( ((i+2) < numChars) && (c=='%') ) {
                      int v = Integer.parseInt(s.substring(i+1,i+3), 16);
                      if (v < 0) {
                         throw new IllegalArgumentException(
                               "Decode: Illegal hex characters in escape (%) pattern - negative value");
                      }

                      bytes[pos++] = (byte) v;
                      i += 3;
                      if (i < numChars) {
                         c = s.charAt(i);
                      }
                   }

                   // A trailing, incomplete byte encoding such as
                   // "%x" will cause an exception to be thrown
                   if ((i < numChars) && (c == '%')) {
                      throw new IllegalArgumentException(
                            "Decode: Incomplete trailing escape (%) pattern");
                   }
                   sb.append(new String(bytes, 0, pos, enc));
                } catch (NumberFormatException e) {
                   throw new IllegalArgumentException(
                         "Decode: Illegal hex characters in escape (%) pattern - " + e.getMessage());
                }
                needToChange = true;
             } else {
                sb.append(c);
                i++;
             }
          }

          return (needToChange ? sb.toString() : s);
       }

       @Override
       public InputStream getResource(String resourceName)
       {
           InputStream is = null;
           if ( this._zipFile != null )
           {
               ZipEntry entry = this._zipFile.getEntry(this._resourcePath + resourceName);
               if( entry == null )
               {
                   Util.getLog().error("Unable to read resource [{}{}]", this._resourcePath, resourceName);
               }
               else
               {
                   try
                   {
                       is = this._zipFile.getInputStream(entry);
                   }
                   catch (IOException e)
                   {
                       Util.getLog().error("Unable to read resource [{}]", this._resourcePath + resourceName, e);
                       is = null;
                   }
               }
           }
           else
           {
               is = this._class.getResourceAsStream(resourceName);
           }
           if(is != null)
           {
               this._streams.add(is);
           }
           return is;
       }

        @Override
        public void close() throws IOException
        {
            if( this._streams != null )
            {
                for(InputStream is : this._streams)
                {
                    try
                    {
                        is.close();
                    }
                    catch(IOException ex)
                    {
                        Util.getLog().error("Closing resource stream thrown exception. Ignoring.", ex);
                    }
                }
                this._streams = null;
            }
            if(this._zipFile != null)
            {
                try
                {
                    this._zipFile.close();
                }
                catch(IOException ex)
                {
                    Util.getLog().error("Closing zip file thrown exception. Ignoring.", ex);
                }
                this._zipFile = null;
            }
        }
   }
   private static class ResourceResolver implements LSResourceResolver
   {
       private final IResourceProvider _resourceProvider;
       public ResourceResolver(IResourceProvider resourceProvider)
       {
           ValidateUtil.validateNotNull(resourceProvider, "resourceProvider");
           this._resourceProvider = resourceProvider;
       }

       @Override
       public LSInput resolveResource(
               String type,
               String namespaceURI,
               String publicId,
               String systemId,
               String baseURI)
       {
           InputStream resourceAsStream = this._resourceProvider.getResource(systemId);
           if ( resourceAsStream == null )
           {
               getLog().debug("Resource [{}] being resolved is null.", systemId);
           }
           else
           {
               getLog().debug("Resources's [{}] InputStream type is: [{};{}]", new Object[] {systemId, resourceAsStream.getClass().getName(), resourceAsStream.toString()});
           }
           return new LSInputImpl(publicId, systemId, baseURI, resourceAsStream);
       }

       private static class LSInputImpl implements LSInput
       {
           private final String publicId;
           private final String systemId;
           private final String baseUri;
           private final InputStream inputStream;

           public LSInputImpl(String publicId, String sysId, String baseURI, InputStream input)
           {
               this.publicId = publicId;
               this.systemId = sysId;
               this.baseUri = baseURI;
               this.inputStream = input;
           }

           @Override
           public String getPublicId()
           {
               return publicId;
           }

           @Override
           public void setPublicId(String publicId)
           {
           }

           @Override
           public String getBaseURI()
           {
               return this.baseUri;
           }

           @Override
           public InputStream getByteStream()
           {
               return this.inputStream;
           }

           @Override
           public boolean getCertifiedText()
           {
               return false;
           }

           @Override
           public Reader getCharacterStream()
           {
               return null;
           }

           @Override
           public String getEncoding()
           {
               return null;
           }

           @Override
           public String getStringData()
           {
               return null;
           }

           @Override
           public void setBaseURI(String baseURI)
           {
           }

           @Override
           public void setByteStream(InputStream byteStream)
           {
           }

           @Override
           public void setCertifiedText(boolean certifiedText)
           {
           }

           @Override
           public void setCharacterStream(Reader characterStream)
           {
           }

           @Override
           public void setEncoding(String encoding)
           {
           }

           @Override
           public void setStringData(String stringData)
           {
           }

           @Override
           public String getSystemId()
           {
               return systemId;
           }

           @Override
           public void setSystemId(String systemId)
           {
           }
       } //end of LSInputImpl
   }
}
