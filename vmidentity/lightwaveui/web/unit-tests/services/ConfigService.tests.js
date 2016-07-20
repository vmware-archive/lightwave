
describe('Unit: Testing Configuration Service - ', function(){

    describe('When I call myApp.configuration', function(){

        var service;


        beforeEach(function() {
            angular.module('myApp.configuration');
        });

        beforeEach(inject(function() {
            var $injector = angular.injector(['myApp.configuration']);
            service = $injector.get('Configuration');
        }));

        it('expect ConfigService to be defined.', function(){
            expect(service).toBeDefined();
        });
    })

});

describe('Unit: Testing Configuration Service Login Token Endpoint - ', function(){

    describe('When I call Configuration.getLoginEndpoint', function(){

        var service;

        beforeEach(function() {
            angular.module('myApp.configuration');
        });

        beforeEach(inject(function() {
            var $injector = angular.injector(['myApp.configuration']);
            service = $injector.get('Configuration');
        }));

        it('expect the token endpoint to be returned.', function(){
            expect(service.getLoginEndpoint("10.160.120.1")).toBe("https://10.160.120.1:443/openidconnect/token");
        });
    })

});


describe('Unit: Testing Configuration Service Member Search Endpoint - ', function(){

    describe('When I call Configuration.getAllUsersEndpoint', function(){

        var service;

        beforeEach(function() {
            angular.module('myApp.configuration');
        });

        beforeEach(inject(function() {
            var $injector = angular.injector(['myApp.configuration']);
            service = $injector.get('Configuration');
        }));

        it('expect the search endpoint to be returned.', function(){
            expect(service.getAllUsersEndpoint("10.160.120.1", "vsphere.local")).toBe("https://10.160.120.1:443/idm/tenant/vsphere.local/search");
        });
    })

});