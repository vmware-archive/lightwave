package com.vmware.vim.sso.admin.exception;

/**
 * This exception is thrown by service methods meaning incorrect provider configuration
 */
public class InvalidProviderException extends ServiceException {

   private static final long serialVersionUID = -5649024920103570953L;
   private final String fieldName;
   private final String fieldValue;

//   public InvalidProviderException(Throwable cause, String fieldName, String fieldValue) {
//      super(getDefaultMessage(fieldName, fieldValue), cause);
//      this.fieldName = fieldName;
//      this.fieldValue = fieldValue;
//   }

   public InvalidProviderException(String fieldName, String fieldValue) {
      super(getDefaultMessage(fieldName, fieldValue));
      this.fieldName = fieldName;
      this.fieldValue = fieldValue;
   }

   private static String getDefaultMessage(String fieldName, String fieldValue) {
      return String.format("Invalid provider configuration: %s = %s", fieldName, fieldValue);
   }

   public String getFieldName() {
      return this.fieldName;
   }

   public String getFieldValue() {
      return this.fieldValue;
   }
}
