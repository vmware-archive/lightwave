package com.vmware.vim.sso.admin.exception;

public class ADDomainAccessDeniedException extends ServiceException {

   private static final long serialVersionUID = 2479251432586421445L;
   private final String domain;
   private final String username;

   public ADDomainAccessDeniedException(String domain, String username) {
      super(getDefaultMessage(domain, username));
      this.domain = domain;
      this.username = username;
   }

   private static String getDefaultMessage(String domain, String username) {
      return String.format("user [%s] cannot access domain [%s]", username, domain);
   }

   public String getDomain() {
      return domain;
   }

   public String getUsername() {
      return username;
   }
}
