/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

/**
 * Person user details. Used as a composite part of {@link PersonUser} object.
 * This class is immutable.
 */
public final class PersonDetails extends PrincipalDetails {

   /**
    * Email address; optional field
    */
   private final String _emailAddress;

   /**
    * User first name; optional field
    */
   private final String _firstName;

   /**
    * User last name; optional field
    */
   private final String _lastName;

   /**
    * User principal name; optional field.
    */
   private final String _userPrincipalName;

   /**
    * Construct person user details
    */
   private PersonDetails(String firstName, String lastName, String userPrincipalName,
      String emailAddress, String description) {
      super(description);

      _firstName = firstName;
      _lastName = lastName;
      _emailAddress = emailAddress;
      _userPrincipalName = userPrincipalName;
   }

   /**
    * Retrieve user's email address
    *
    * @return the email address or {@code null} if not set
    */
   public String getEmailAddress() {
      return _emailAddress;
   }

   /**
    * Retrieve user's first name
    *
    * @return first name or {@code null} if not specified
    */
   public String getFirstName() {
      return _firstName;
   }

   /**
    * Retrieve user's last name
    *
    * @return last name or {@code null} if not specified
    */
   public String getLastName() {
      return _lastName;
   }

   /**
    * Retrieve user principal name.
    * @return
    */
   public String getUserPrincipalName()
   {
       return this._userPrincipalName;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected Object[] getIdentityState() {
      return new Object[] { getDescription(), getEmailAddress(),
         getFirstName(), getLastName(), getUserPrincipalName() };
   }

   public static class Builder {

      private String _emailAddress;
      private String _firstName;
      private String _lastName;
      private String _description;
      private String _userPrincipalName;

      public Builder() {
      }

      /**
       * @param emailAddress
       *           the user's email address
       */
      public Builder setEmailAddress(String emailAddress) {
         _emailAddress = emailAddress;

         return this;
      }

      /**
       * @param firstName
       *           the user's first name
       */
      public Builder setFirstName(String firstName) {
         _firstName = firstName;

         return this;
      }

      /**
       * @param lastName
       *           the user's last name
       */
      public Builder setLastName(String lastName) {
         _lastName = lastName;

         return this;
      }

      /**
       * @param userPrincipalName
       *           the user's principal name (UPN)
       */
      public Builder setUserPrincipalName(String userPrincipalName) {
         _userPrincipalName = userPrincipalName;

         return this;
      }

      /**
       * @param description
       *           the user's description
       */
      public Builder setDescription(String description) {
         _description = description;

         return this;
      }

      public PersonDetails createPersonDetails() {
         return new PersonDetails(_firstName, _lastName,
             _userPrincipalName, _emailAddress, _description);
      }
   }
}
