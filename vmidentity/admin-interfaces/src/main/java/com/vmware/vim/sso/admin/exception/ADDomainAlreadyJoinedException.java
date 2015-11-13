package com.vmware.vim.sso.admin.exception;

public class ADDomainAlreadyJoinedException extends ServiceException
{
   private static final long serialVersionUID = 4244846002175182135L;

   public ADDomainAlreadyJoinedException(){

      super("SSO server is already joined to AD domain");
   }
}
