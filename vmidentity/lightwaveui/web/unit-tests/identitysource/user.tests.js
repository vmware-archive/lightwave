/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
 
describe('IdentitySourceCntrl', function() {
    var scope, $location, createController, IdentitySourceServiceMock, MemberServiceMock, UserServiceMock;

    beforeEach(function() {
        module('ngRoute');
        module('myApp.identitysource')
    });

    beforeEach(function(){
        IdentitySourceServiceMock = {
            GetAll : function() {
                return {
                    then: function (callback) {
                        return callback({
                            "data": [
                                {
                                    "domainType": "SYSTEM_DOMAIN",
                                    "name": "vsphere.local"
                                },
                                {
                                    "domainType": "LOCAL_OS_DOMAIN",
                                    "name": "localos"
                                }
                            ],
                            "status": 200
                        });
                    }
                }
            }
        },

        MemberServiceMock = {
            Search : function() {
                return {
                    then: function (callback) {
                        return callback({
                            data: {
                                users: [
                                    {
                                        "name": "Administrator",
                                        "domain": "vsphere.local",
                                        "details": {
                                            "upn": "Administrator@VSPHERE.LOCAL",
                                            "firstName": "Administrator",
                                            "lastName": "vsphere.local"
                                        },
                                        "disabled": false,
                                        "locked": false
                                    }
                                ]
                            }
                        });
                    }
                }
            }
        },

        UserServiceMock = {
            Get: function () {

            },
            Add: function () {

            },
            Delete: function () {

            },
            Update: function () {

            }
        }
    });

    beforeEach(inject(function ($rootScope, $controller, _$location_) {
        $location = _$location_;
        scope = $rootScope.$new();

        $rootScope.globals = {
            currentUser: {
                server: "10.160.1.2",
                tenant: "vsphere.local",
                username: "Administrator",
                password: "dummy",
                token : {
                    "access_token": "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJhdWQiOlsiYWRtaW5pc3RyYXRvckB2c3BoZXJlLmxvY2FsIiwicnNfYWRtaW5fc2VydmVyIl0sInNjb3BlIjoiYXRfZ3JvdXBzIHJzX2FkbWluX3NlcnZlciBvcGVuaWQgb2ZmbGluZV9hY2Nlc3MgaWRfZ3JvdXBzIiwiaXNzIjoiaHR0cHM6XC9cL3NjLXJkb3BzLXZtMDItZGhjcC0zNS0xNTcuZW5nLnZtd2FyZS5jb21cL29wZW5pZGNvbm5lY3RcL3ZzcGhlcmUubG9jYWwiLCJncm91cHMiOlsidnNwaGVyZS5sb2NhbFxcVXNlcnMiLCJ2c3BoZXJlLmxvY2FsXFxBZG1pbmlzdHJhdG9ycyIsInZzcGhlcmUubG9jYWxcXENBQWRtaW5zIiwidnNwaGVyZS5sb2NhbFxcQ29tcG9uZW50TWFuYWdlci5BZG1pbmlzdHJhdG9ycyIsInZzcGhlcmUubG9jYWxcXFN5c3RlbUNvbmZpZ3VyYXRpb24uQmFzaFNoZWxsQWRtaW5pc3RyYXRvcnMiLCJ2c3BoZXJlLmxvY2FsXFxTeXN0ZW1Db25maWd1cmF0aW9uLkFkbWluaXN0cmF0b3JzIiwidnNwaGVyZS5sb2NhbFxcTGljZW5zZVNlcnZpY2UuQWRtaW5pc3RyYXRvcnMiLCJ2c3BoZXJlLmxvY2FsXFxFdmVyeW9uZSJdLCJ0b2tlbl9jbGFzcyI6ImFjY2Vzc190b2tlbiIsInRva2VuX3R5cGUiOiJCZWFyZXIiLCJleHAiOjE0NjUwMzU5OTYsImlhdCI6MTQ2NTAzNTY5NiwianRpIjoiUkhINC1PdVVJSjlxdFlWNG43b0VtN1FsRXItMVk4eTJ0aTV1UmVHUlhPSSIsInRlbmFudCI6InZzcGhlcmUubG9jYWwiLCJhZG1pbl9zZXJ2ZXJfcm9sZSI6IkFkbWluaXN0cmF0b3IifQ.jJ6Pd5e-iIyDjp0qRJS02lodP8r-Xn1dD1rXoRAZFZoT9tDmwSbnMzqDVpUCgWm8ZlkLQCCCwXCS5iXiPEHYl3s6YrY3evpB4BnXJfUbOS641f7C0MguMRKXWB1jVcndanAIuqK413TKqYRMo6G2CLjU83KBX1ScnLcGDG6eaKvc6ZjYL4bbUhr0qAUvV4VZq3rXi4ju6RKWE5dIUQO_zHDUUOnPqIAvXo_9PPAdks0Kbgaze_NcY7HA4KOSdl-c8uilKd9sNlWaCDqqfC0YeG-vlAIaNzV3_jT-6u4tRySOanfnnRdZZZ-KCmnHvEa-vvIRg4KVI-cqcUmOtYTdQQ",
                    "refresh_token": "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJhdWQiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJzY29wZSI6ImF0X2dyb3VwcyByc19hZG1pbl9zZXJ2ZXIgb3BlbmlkIG9mZmxpbmVfYWNjZXNzIGlkX2dyb3VwcyIsImlzcyI6Imh0dHBzOlwvXC9zYy1yZG9wcy12bTAyLWRoY3AtMzUtMTU3LmVuZy52bXdhcmUuY29tXC9vcGVuaWRjb25uZWN0XC92c3BoZXJlLmxvY2FsIiwidG9rZW5fY2xhc3MiOiJyZWZyZXNoX3Rva2VuIiwidG9rZW5fdHlwZSI6IkJlYXJlciIsImV4cCI6MTQ2NTA1NzI5NiwiaWF0IjoxNDY1MDM1Njk2LCJqdGkiOiJyZVlsSE9kVnQzdjhhOEhxMHdqSE5TMmdXcHpIbjFIUTBzQnJHUlVaaWpjIiwidGVuYW50IjoidnNwaGVyZS5sb2NhbCJ9.j0uvygEYHYgssypyGjALHKhrmJsBCop83SS36axFEMgEZ4tp84y0GlgPaSo5EaXy2aesjl8gmXBOOZeBlXCIWjFE-Xb-GCjDuyd9-ABYyce2-zDaM9yE300WUBz-OVJD94hdM2wYyhsfHR608XkPILlxcoBqAVUsiyhGyWNvNkiiSftdwd43EF5d-noyWRpclCirXU0mmWOlaMKLuHwH3dt9rBT9ow1UkJL5BysR4VSn8ntWMbjwMbSqBdnnMlcIywIjum6wW7Z52ofZWda-UoFP7IN_4mDzc6aIrp9BdM8YWmThP5NEAXUMlTgC807pOJGms1IXjnaqvlGOv95zew",
                    "id_token": "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJpc3MiOiJodHRwczpcL1wvc2MtcmRvcHMtdm0wMi1kaGNwLTM1LTE1Ny5lbmcudm13YXJlLmNvbVwvb3BlbmlkY29ubmVjdFwvdnNwaGVyZS5sb2NhbCIsImdyb3VwcyI6WyJ2c3BoZXJlLmxvY2FsXFxVc2VycyIsInZzcGhlcmUubG9jYWxcXEFkbWluaXN0cmF0b3JzIiwidnNwaGVyZS5sb2NhbFxcQ0FBZG1pbnMiLCJ2c3BoZXJlLmxvY2FsXFxDb21wb25lbnRNYW5hZ2VyLkFkbWluaXN0cmF0b3JzIiwidnNwaGVyZS5sb2NhbFxcU3lzdGVtQ29uZmlndXJhdGlvbi5CYXNoU2hlbGxBZG1pbmlzdHJhdG9ycyIsInZzcGhlcmUubG9jYWxcXFN5c3RlbUNvbmZpZ3VyYXRpb24uQWRtaW5pc3RyYXRvcnMiLCJ2c3BoZXJlLmxvY2FsXFxMaWNlbnNlU2VydmljZS5BZG1pbmlzdHJhdG9ycyIsInZzcGhlcmUubG9jYWxcXEV2ZXJ5b25lIl0sInRva2VuX2NsYXNzIjoiaWRfdG9rZW4iLCJ0b2tlbl90eXBlIjoiQmVhcmVyIiwiZ2l2ZW5fbmFtZSI6IkFkbWluaXN0cmF0b3IiLCJhdWQiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJzY29wZSI6ImF0X2dyb3VwcyByc19hZG1pbl9zZXJ2ZXIgb3BlbmlkIG9mZmxpbmVfYWNjZXNzIGlkX2dyb3VwcyIsImV4cCI6MTQ2NTAzNTk5NiwiaWF0IjoxNDY1MDM1Njk2LCJmYW1pbHlfbmFtZSI6InZzcGhlcmUubG9jYWwiLCJqdGkiOiJVN1Bkd0NOc0Q2RDVmZWt0eDBMV1I3WFRXY3JZdkxWanhXa0U3VFM3WHFrIiwidGVuYW50IjoidnNwaGVyZS5sb2NhbCJ9.mTNi3JC6BTr36nSthTIrr5wiGtJ_1wKgtbcqKhsQX_vuPk0BywnykqCg5BrdMltLTEn9vcFv4qpCipdfg_17tkAB5vD8W8ANTFx3Eby0msDMUOgibIlHW0N1dcYvrZgHdlPDgauXs1Mdf2xcfGTv-EJ5hu6qAwQ2UyZBZanKKl7htt9ORvYG8GNq_P5I6wBb0dr2FP1AZ_RiEzhnuV7Ih4SomTv8lOSIUssl0vkHkeEMni70CiHuvg9NJByRuFOEIDmr4NdCmDmUQF7tYlEEHDVG59pboFsDjyhsKCDr5Z6rV8ZuhE3D9U4fgoY0c1bxgpljNcZNqsz1Cc4RIjVbzg",
                    "token_type": "Bearer",
                    "expires_in": 300
                }
            }
        };
        createController = function() {
            return $controller('IdentitySourceCntrl', {
                '$scope' : scope,
                '$rootScope' : scope,
                'IdentitySourceService' : IdentitySourceServiceMock,
                'MemberService' : MemberServiceMock,
                'UserService' : UserServiceMock
            });
        };
    }));

    it('should have identity sources, system identity source and users loaded on init', function() {
        var controller = createController();
        $location.path('/identitysourceusers');
        expect($location.path()).toBe('/identitysourceusers');
        expect(scope.vm.identitysources.length).toBe(2);
        expect(scope.vm.identitysource.name).toBe('vsphere.local');
        expect(scope.vm.users.length).toBe(1);
    });
});