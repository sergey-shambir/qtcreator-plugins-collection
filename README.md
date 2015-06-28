qtcreator-plugins-collection
============================

Collection of plugins adapted for the QtCreator 3.1 from Ubuntu Trusty. Should be compatible with version from <a href="https://launchpad.net/~ubuntu-sdk-team/+archive/ppa">ubuntu sdk ppa</a>.

## GoLang
This plugin provided by Canonical, Ltd. under LGPL license. It contains additional patches to allow development of non-QML Go projects.

License: LGPL + Qt exception

## GoEditor
Provides editor with code completion for Golang
* code completion needs /usr/bin/gocode, see https://github.com/nsf/gocode
* semantic highlighting needs /usr/bin/gosemki, see https://github.com/sergey-shambir/gosemki
* auto-formatting code on save needs gofmt installed and added to $PATH.

Author: Sergey Shambir.

License: MIT

## ClangCodeModel (outdated)
Enables clang parser for code completion, errors displaying and highlighting. Known issue: memory leaks on editing, memory consumption can grown 3-6 times.

Author: Erik Verbruggen, Digia plc.

See also http://qt-project.org/wiki/Branches

## VCProjectManager (outdated)
Allows to open Visual Studio 2005-2008 project files in readonly mode.

Authors: Radovan Zivkovic, Bojan Petrovic

See also https://codereview.qt-project.org/#q,status:merged+project:qt-creator/qt-creator+branch:wip/vcproj,n,z

License: LGPL + Qt exception

## XCodeProjectManager (outdated)
Allows to open XCode project files in readonly mode.

Author: Sergey Shambir.

License: MIT
