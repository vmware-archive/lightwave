package com.vmware.vim.sso.admin.exception;

public class ADIDSAlreadyExistException extends ADIDPRegistrationServiceException {

   /**
    * Domain name of the duplicated native AD IDS or LDAP AD IDS.
    */
   final String domainName;
   private static final long serialVersionUID = -4284865881381641922L;

   public ADIDSAlreadyExistException(String message, String name) {
      super(message);
      this.domainName = name;
   }

   public ADIDSAlreadyExistException(String domainName) {
      super(getDefaultMessage(domainName));
      assert (null != domainName && !domainName.trim().isEmpty());
      this.domainName = domainName;
   }

   public ADIDSAlreadyExistException(Throwable t, String domainName) {
      super(getDefaultMessage(domainName), t);
      assert (null != domainName && !domainName.trim().isEmpty());
      this.domainName = domainName;
   }

   public String getDomainName() {
      return domainName;
   }

   private static String getDefaultMessage(String name) {
      return String.format(
         "There is already one IdentitySource of AD type registered: name '%s'. Only one IDS of AD type is allowed",
         name);
   }
}
