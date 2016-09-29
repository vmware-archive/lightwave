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
 
var gulp = require('gulp'),
    rename = require('gulp-rename'),
    concat = require('gulp-concat'),
    clean = require('gulp-clean'),
    //uglify = require('gulp-uglify'),
    htmlmin = require('gulp-html-minifier'),
    cleanCSS = require('gulp-clean-css');

var version = '1.0.2.0';

var sequence = [
    'lightwave-app-ui-js',
    'lightwave-ui-js-minify',
    'lightwave-ui-vendor-js-minify',
    'lightwave-ui-html-minify',
    'lightwave-ui-vendor-css-minify',
    'lightwave-ui-css-minify',
    'lightwave-ui-copy-assets'
];

delete(['./dist/*']);

gulp.task('default', sequence);


gulp.task('lightwave-ui-js-minify', function() {
    var app_js = 'lightwave-ui.' + version + '.js';
    var dest_js_folder = './dist/js';

    gulp.src([
            './app/src/*.js',
            './app/src/**/*.js',
            './app/src/**/**/*.js',
            './app/src/**/**/**/*.js'
        ])
        .pipe(concat(app_js))
        //.pipe(uglify())
        .pipe(rename({ extname: '.min.js' }))
        .pipe(gulp.dest(dest_js_folder));
});

gulp.task('lightwave-app-ui-js', function() {
    var app_js = 'lightwave-app-ui.' + version + '.js';
    var dest_js_folder = './dist/js';

    gulp.src(['./app/app.js'])
        .pipe(concat(app_js))
        .pipe(gulp.dest(dest_js_folder));
});

gulp.task('lightwave-ui-vendor-js-minify', function() {
    var app_js = 'lightwave-ui-vendor.' + version + '.js';
    var dest_js_folder = './dist/js';

    gulp.src([
            './app/bower_components/jquery/dist/jquery.min.js',
            './app/bower_components/angular/angular.js',
            './app/bower_components/angular-bootstrap/ui-bootstrap.min.js',
            './app/bower_components/angular-bootstrap/ui-bootstrap-tpls.min.js',
            './app/bower_components/angular-cookies/angular-cookies.js',
            './app/bower_components/ng-dialog/js/ngDialog.min.js',
            './app/bower_components/angular-route/angular-route.js',
            './app/bower_components/cryptojslib/components/core-min.js',
            './app/bower_components/cryptojslib/components/sha1-min.js',
            './app/bower_components/kjur-jsrsasign/min/base64x-1.1.min.js',
            './app/bower_components/kjur-jsrsasign/min/jws-3.2.min.js',
            './app/bower_components/kjur-jsrsasign/ext/jsbn.js',
            './app/bower_components/kjur-jsrsasign/ext/jsbn2.js',
            './app/bower_components/kjur-jsrsasign/ext/rsa.js',
            './app/bower_components/kjur-jsrsasign/ext/rsa2.js',
            './app/bower_components/kjur-jsrsasign/ext/sha512.js',
            './app/bower_components/kjur-jsrsasign/ext/base64.js',
            './app/bower_components/kjur-jsrsasign/min/crypto-1.1.min.js',
            './app/bower_components/kjur-jsrsasign/min/asn1hex-1.1.min.js',
            './app/bower_components/kjur-jsrsasign/min/rsapem-1.1.min.js',
            './app/bower_components/kjur-jsrsasign/min/rsasign-1.2.min.js',
            './app/bower_components/kjur-jsrsasign/min/x509-1.1.min.js'
        ])
        .pipe(concat(app_js))
        //.pipe(uglify())
        .pipe(rename({ extname: '.min.js' }))
        .pipe(gulp.dest(dest_js_folder));
});


gulp.task('lightwave-ui-html-minify', function() {
    gulp.src([
        './app/src/**/*.html',
        './app/*.html',
        './app/src/sso/**/*.html',
        './app/src/shared/**/*.html'
        ])
        .pipe(htmlmin({collapseWhitespace: true}))
        .pipe(gulp.dest('./dist'))
});

gulp.task('lightwave-ui-css-minify', function() {
    var app_css = 'lightwave-ui.' + version + '.min.css';
    gulp.src('./app/app.css')
        .pipe(cleanCSS({compatibility: 'ie8'}))
        .pipe(rename(app_css))
        .pipe(gulp.dest('./dist/css'));
});

gulp.task('lightwave-ui-vendor-css-minify', function() {
    var app_vendor_css = 'lightwave-ui-vendor.' + version + '.css';
    gulp.src([
            './app/bower_components/ng-dialog/css/ngDialog.min.css',
            './app/bower_components/ng-dialog/css/ngDialog-theme-default.min.css'
        ])
        .pipe(concat(app_vendor_css))
        .pipe(cleanCSS({compatibility: 'ie8'}))
        .pipe(rename({ extname: '.min.css' }))
        .pipe(gulp.dest('./dist/css'));
});

gulp.task('lightwave-ui-copy-assets', function() {
    gulp.src(['./app/assets/*.png','./app/assets/*.gif'])
        .pipe(gulp.dest('./dist/assets'));
});