package com.vmware.vim.sso.admin.exception;

/**
 * This exception is thrown by service methods indicating that host is not
 * joined the required domain
 */
public class HostNotJoinedRequiredDomainException extends ADIDPRegistrationServiceException {

   private static final long serialVersionUID = 7774656340218069416L;
   private final String requiredDomainName;
   private final String joinedDomainName;

   public HostNotJoinedRequiredDomainException(String requiredDomainName,
         String joinedDomainName) {
      super(getDefaultMessage(requiredDomainName, joinedDomainName));
      assert (null != requiredDomainName && !requiredDomainName.trim().isEmpty());
      this.requiredDomainName = requiredDomainName;
      this.joinedDomainName = joinedDomainName;
   }

   public HostNotJoinedRequiredDomainException(String requiredDomainName,
         String joinedDomainName, Throwable t) {
      super(getDefaultMessage(requiredDomainName, joinedDomainName), t);
      assert (null != requiredDomainName && !requiredDomainName.trim().isEmpty());
      this.requiredDomainName = requiredDomainName;
      this.joinedDomainName = joinedDomainName;
   }

   private static String getDefaultMessage(String requiredDomanName,
         String joinedDomainName) {
      if (joinedDomainName == null || joinedDomainName.isEmpty())
      {
          return "To support native AD, the host is required to join properly.";
      }

      return String.format(
            "The host is required to join to domain [%s] but joined to [%s]",
            requiredDomanName, joinedDomainName);
   }

   public String getRequiredDomainName() {
      return requiredDomainName;
   }

   public String getJoinedDomainName() {
      return joinedDomainName;
   }
}
