/*
 *
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
 *
 */

package com.vmware.identity.idm.server.provider.activedirectory;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

public class SecurityIdentifier
{
	private static int SID_REVISION            =  1;
	private static int SID_MAX_SUB_AUTHORITIES = 15;
	
	private static int IDENTIFIER_AUTHORITY_BYTE_COUNT = 6;
	private static int SUB_AUTHORITY_BYTE_COUNT        = 4;

	private byte _revision;
	private byte _subAuthorityCount;
	private long _identifierAuthority;
	private List<Long> _subAuthorities;
	
	protected 
	SecurityIdentifier(
		byte       revision, 
		byte       subAuthorityCount, 
		long       identifierAuthority, 
		List<Long> subAuthorities
		)
	{
		_revision = revision;
		_subAuthorityCount = subAuthorityCount;
		_identifierAuthority = identifierAuthority;
		_subAuthorities = subAuthorities;
	}
	
	public int getRevision()
	{
		return _revision;
	}
	
	public int getSubAuthorityCount()
	{
		return _subAuthorityCount;
	}
	
	public long getIdentifierAuthority()
	{
		return _identifierAuthority;
	}
	
	public Collection<Long> getSubAuthorities()
	{
		return Collections.unmodifiableList((List<? extends Long>) _subAuthorities);
	}
	
	/**
	 * Set the Relative Identifier (RID)
	 * 
	 * The RID is an unsigned 32 bit integer and is always the last 
	 * sub-authority in the Security Identifier.
	 * 
	 * @param value value of the relative identifier
	 */
	public void setRID(long value)
	{
		if ((value < 0) || (value > 4294967295L))
		{
			throw new InvalidParameterException("RID Value is out of bounds");
		}
		
		if (_subAuthorities.size() == 0)
		{
			_subAuthorityCount = 1;
			_subAuthorities = new ArrayList<Long>(1);
			_subAuthorities.add(new Long(value));
		}
		else
		{
			_subAuthorities.set(_subAuthorityCount-1, new Long(value));
		}
	}
	
	public byte[] toArray()
	{
		int length = Byte.SIZE + // Revision
				     Byte.SIZE + // sub authority count
				 	 IDENTIFIER_AUTHORITY_BYTE_COUNT +
		             ((int)_subAuthorityCount) * SUB_AUTHORITY_BYTE_COUNT;
		
		byte[] sid = new byte[length];
		int    index = 0;
		
		sid[index++] = _revision;
		sid[index++] = _subAuthorityCount;
		
		// IDENTIFIER_AUTHORITY
		for (int i = IDENTIFIER_AUTHORITY_BYTE_COUNT -1; i >= 0; i--)
		{
			sid[index++] = (byte)((_identifierAuthority & (1 << i)) >> i);
		}
		
		for (Long subAuthority : _subAuthorities)
		{
			long val = subAuthority.longValue();
			
			for (int j = SUB_AUTHORITY_BYTE_COUNT-1; j >= 0; j--)
			{
				sid[index++] = (byte)((val & (1 << j)) >> j);
			}
		}
		
		return sid;
	}
	
	public String toString()
	{
		StringBuffer sb = new StringBuffer("S-");
		
		sb.append(getRevision());
		sb.append("-");
		sb.append(getIdentifierAuthority());
		
		Collection<Long> subAuthorities = getSubAuthorities();
		
		for (Long authority : subAuthorities)
		{
			sb.append("-");
			sb.append(authority.toString());
		}
		
		return sb.toString();
	}
	
	public static SecurityIdentifier build(byte[] sid)
	{
		if (sid == null)
		{
			throw new InvalidParameterException("SID is null.");
		}
		
		int index = 0;
		int remaining = sid.length;
		
		if (remaining < 1)
		{
			throw new InvalidParameterException("Invalid SID Length");
		}
		
		byte revision = sid[index++];
		
		if (revision != SID_REVISION)
		{
			throw new InvalidParameterException("Invalid SID revision");
		}
		
		if (--remaining < 1)
		{
			throw new InvalidParameterException("Invalid SID Length");
		}
		
		byte subAuthorityCount = sid[index++];
		
		if (subAuthorityCount > SID_MAX_SUB_AUTHORITIES)
		{
			throw new InvalidParameterException(
							"Invalid number of sub-authorities in SID");
		}
		
		if (--remaining < IDENTIFIER_AUTHORITY_BYTE_COUNT)
		{
			throw new InvalidParameterException("Invalid SID Length");
		}
		
		long identifierAuthority = 0;
		{
			byte[] longBuf = new byte[] { 0, 0, 0, 0, 0, 0, 0, 0 };
				
			for (int i = IDENTIFIER_AUTHORITY_BYTE_COUNT-1; i >= 0; i--)
			{
				longBuf[i] = sid[index++];
			}
			
			remaining -= IDENTIFIER_AUTHORITY_BYTE_COUNT;
			
			ByteBuffer byteBuf = ByteBuffer.wrap(longBuf);
			
			byteBuf.order(ByteOrder.LITTLE_ENDIAN);
			
			identifierAuthority = byteBuf.getLong();
		}
		
		int expectedLength = subAuthorityCount * SUB_AUTHORITY_BYTE_COUNT;
		
		if (remaining != expectedLength)
		{
			throw new InvalidParameterException("Invalid SID Length");
		}
		
		List<Long> subAuthorities = new ArrayList<Long>();
		
		if (subAuthorityCount > 0)
		{	
			byte[] longBuf = new byte[] { 0, 0, 0, 0, 0, 0, 0, 0 };
			
			for (int i = 0; i < subAuthorityCount; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					longBuf[j] = sid[index++];
				}
				
				ByteBuffer byteBuf = ByteBuffer.wrap(longBuf);
				
				byteBuf.order(ByteOrder.LITTLE_ENDIAN);
				
				subAuthorities.add(new Long(byteBuf.getLong()));
			}
		}

		return new SecurityIdentifier(
						revision,
						subAuthorityCount,
						identifierAuthority,
						subAuthorities);
	}
}
