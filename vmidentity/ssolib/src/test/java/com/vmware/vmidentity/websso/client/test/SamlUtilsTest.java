package com.vmware.vmidentity.websso.client.test;

import java.util.Calendar;
import java.util.Date;

import org.joda.time.DateTime;
import org.junit.Assert;
import org.junit.Test;
import org.opensaml.xml.validation.ValidationException;

import com.vmware.identity.websso.client.SamlUtils;

public class SamlUtilsTest {

    private int clockTolerance = 10; // in seconds
    Exception ex;

    /**
     * It checks the positive cases for ValidateSessionExpiry()
     */
    @Test
    public final void testValidateSesssionExpiry_success() {

        // Case1: Validation should pass with positive clockTolerance
        DateTime date = new DateTime(new Date());
        try {
            SamlUtils.ValidateSessionExpiry(date, clockTolerance);
        } catch (Exception e) {
            ex = e;
        }
        Assert.assertNull(ex);

        // Case2: Validation should pass with some future date/time
        Calendar c = Calendar.getInstance();
        c.add(Calendar.YEAR, 100);
        date = new DateTime(c.getTime());
        try {
            SamlUtils.ValidateSessionExpiry(date, clockTolerance);
        } catch (ValidationException e) {
            ex = e;
        }

        Assert.assertNull(ex);
    }

    /**
     * It checks the negative cases for ValidateSessionExpiry()
     */
    @Test
    public final void testValidateSesssionExpiry_failure() {

        // Case1: Validation should fail with zero clock tolerance
        DateTime date = new DateTime(new Date());
        try {
            SamlUtils.ValidateSessionExpiry(date, 0);
        } catch (Exception e) {
            ex = e;
        }
        Assert.assertNotNull(ex);

        // Case2: Validation should fail with any past date/time.
        Calendar c = Calendar.getInstance();
        c.add(Calendar.DATE, -5);
        date = new DateTime(c.getTime());

        try {
            SamlUtils.ValidateSessionExpiry(date, clockTolerance);
        } catch (ValidationException e) {
            ex = e;
        }

        Assert.assertNotNull(ex);
    }
}
