package com.vmware.vim.sso.admin.exception;

public class DomainManagerException extends ADIDPRegistrationServiceException {

   private static final long serialVersionUID = -5820870200173702476L;
   private final String domainName;
   private final int errorCode;

   public DomainManagerException(String name, int errorCode, Throwable cause) {
      super(getDefaultMessage(name, errorCode), cause);
      assert (null != name && !name.trim().isEmpty());
      this.domainName = name;
      this.errorCode = errorCode;
   }

   public DomainManagerException(String name, int errorCode) {
      super(getDefaultMessage(name, errorCode));
      assert (null != name && !name.trim().isEmpty());
      this.domainName = name;
      this.errorCode = errorCode;
   }

   private static String getDefaultMessage(String domainName, int errorCode) {
      return String.format(
            "DomainManager native API error with domainName [%s], errorCode [%d]", domainName, errorCode);
   }

   public String getDomainName() {
      return domainName;
   }

   public int getErrorcode() {
      return errorCode;
   }
}
