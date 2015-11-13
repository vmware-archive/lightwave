package com.vmware.vim.sso.admin.exception;

public class ADDomainUnknownDomainException extends ServiceException {

   private static final long serialVersionUID = -4202893031916453866L;
   private final String domain;

   public ADDomainUnknownDomainException(String domain)
   {
      super(getDefaultMessage(domain));
      this.domain = domain;
   }

   private static String getDefaultMessage(String domain){
      return String.format("cannot contact domain [%s]", domain);
   }

   public String getDomain() {
      return domain;
   }

}
