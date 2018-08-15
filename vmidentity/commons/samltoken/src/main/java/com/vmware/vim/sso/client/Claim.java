package com.vmware.vim.sso.client;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.vmware.identity.token.impl.ValidateUtil;

public class Claim {

    private final String _name;
    private final String _nameFormat;
    private final String _friendlyName;
    private final List<String> _value;

    /**
     * Creates a claim.
     *
     * @param name
     *           The name of the attribute. Cannot be <code>null</code>
     * @param nameFormat
     *           The format of the attribute name.  Cannot be <code>null</code>
     */
    public Claim(String name, String nameFormat) {
       this(name, nameFormat, new ArrayList<String>());
    }

    /**
     * Creates a claim.
     *
     * @param name
     *           The name of the claim. Cannot be <code>null</code>
     * @param nameFormat
     *           The format of the attribute name. Cannot be <code>null</code>
     * @param value
     *           List of values representing the claim attribute values.
     */
    public Claim(String name, String nameFormat, List<String> value) {
       this(name, nameFormat, null, value);
    }

    /**
     * Creates a claim.
     *
     * @param name
     *           The name of the attribute. Cannot be <code>null</code>
     * @param nameFormat
     *           The format of the attribute name. Cannot be <code>null</code>
     * @param friendlyName
     *           a friendly name of the attribute.
     * @param value
     *           List of values representing the claim attribute values.
     */
    public Claim(String name, String nameFormat, String friendlyName, List<String> value) {
       ValidateUtil.validateNotNull(name, "Claim name");
       ValidateUtil.validateNotNull(nameFormat, "Name format");

       _name = name;
       _nameFormat = nameFormat;
       _friendlyName=  friendlyName;
       if (value != null) {
           _value = Collections.unmodifiableList(value);
       } else {
           _value = Collections.emptyList();
       }
    }

    /**
     * @return The name of the attribute. Cannot be <code>null</code>
     */
    public String getName() {
       return _name;
    }

    /**
     * @return The name format. Cannot be <code>null</code>
     */
    public String getNameFormat() {
       return _nameFormat;
    }

    /**
     * @return the friendlyName
     */
    public String getFriendlyName() {
       return _friendlyName;
    }

    /**
     * @return List of values representing the claim attribute values
     *         Cannot be <code>null</code>
     */
    public List<String> getValue() {
       return _value;
    }

    @Override
    public int hashCode() {
       return _name.hashCode() + _nameFormat.hashCode() + _value.hashCode()
              + ((_friendlyName != null) ? _friendlyName.hashCode() : 0);
    }

    @Override
    public boolean equals(Object o) {
       if (o == this) {
          return true;
       }
       if (o instanceof Claim) {
          Claim c = (Claim) o;
          return _name.equals(c._name)
             && _nameFormat.equals(c._nameFormat)
             && _value.equals(c._value)
             &&
             (   // either both null or equal
                 ( ( _friendlyName == null ) && (c._friendlyName == null) )
                 ||
                 ( ( _friendlyName != null ) && (_friendlyName.equals(c._friendlyName) ) )
             );
       }
       return false;
    }
}
