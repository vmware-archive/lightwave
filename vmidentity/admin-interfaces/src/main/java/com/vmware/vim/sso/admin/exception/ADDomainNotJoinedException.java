package com.vmware.vim.sso.admin.exception;

public class ADDomainNotJoinedException extends ServiceException
{
   private static final long serialVersionUID = -1814439672854502826L;

   public ADDomainNotJoinedException() {

      super("SSO server is not joined to AD domain");
   }
}
