// Karma configuration
// Generated on Sat Jun 04 2016 12:35:27 GMT+0530 (IST)

module.exports = function(config) {
  config.set({

    // base path that will be used to resolve all patterns (eg. files, exclude)
    basePath: '',


    // frameworks to use
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['jasmine'],


    // list of files / patterns to load in the browser
    files: [
        'app/bower_components/angular/angular.js',
        'app/bower_components/angular-route/angular-route.js',
        'node_modules/angular-mocks/angular-mocks.js',
        //'app/bower_components/jquery/dist/jquery.min.js',
        //'app/bower_components/angular-bootstrap/ui-bootstrap.min.js',
        //'app/bower_components/angular-bootstrap/ui-bootstrap-tpls.min.js',
        //'app/bower_components/angular-route/angular-route.js',
        'app/services/*.js',
        'app/identitysource/*.js',
        'app/certificates/*.js',
        'app/components/*.js',
        'app/diagnostics/*.js',
        'app/home/*.js',
        'app/login/*.js',
        'app/policies/*.js',
        'app/servermgmt/*.js',
        'app/serviceproviders/*.js',
        'unit-tests/**/*.js'
    ],


    // list of files to exclude
    exclude: [
		'app/bower_components',
        'app/assets'
    ],


    // preprocess matching files before serving them to the browser
    // available preprocessors: https://npmjs.org/browse/keyword/karma-preprocessor
    preprocessors: {
        'app/**/*':'coverage'
    },


    // test results reporter to use
    // possible values: 'dots', 'progress'
    // available reporters: https://npmjs.org/browse/keyword/karma-reporter
    reporters: ['progress', 'coverage'],


    coverageReporter: {
          type : 'html',
          // output coverage reports
          dir : 'coverage/'
      },


    // web server port
    port: 9876,


    // enable / disable colors in the output (reporters and logs)
    colors: true,


    // level of logging
    // possible values: config.LOG_DISABLE || config.LOG_ERROR || config.LOG_WARN || config.LOG_INFO || config.LOG_DEBUG
    logLevel: config.LOG_INFO,


    // enable / disable watching file and executing tests whenever any file changes
    autoWatch: true,


    // start these browsers
    // available browser launchers: https://npmjs.org/browse/keyword/karma-launcher
    browsers: ['Chrome'],


    // Continuous Integration mode
    // if true, Karma captures browsers, runs the tests and exits
    singleRun: false
  })
}
