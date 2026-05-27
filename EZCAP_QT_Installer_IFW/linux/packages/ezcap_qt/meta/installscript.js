/**************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
**************************************************************************/

function Component()
{
}

Component.prototype.createOperations = function () {
    try {
        component.createOperations();
	//windows平台的操作
        if (installer.value("os") === "win") {
            component.addOperation("CreateShortcut", "@TargetDir@/EZCAP.exe", "@DesktopDir@/EZCAP.lnk");
        }
	//Linux平台下的操作
        if (installer.value("os") === "x11") {
    //创建桌面文件@TargetDir是安装目录的变量，EZCAP.desktop是桌面文件的文件名
            component.addOperation("CreateDesktopEntry", "@TargetDir@/EZCAP.desktop", "Encoding=UTF-8\nVersion=1.0\nType=Application\nTerminal=false\nExec=@TargetDir@/EZCAP\nName=EZCAP_QT\nIcon=@TargetDir@/EZCAP.png");
	//获取当前桌面路径    
	var desktoppath = QDesktopServices.storageLocation(0);
	//将桌面文件复制到/usr/share/applications/	目录下
            component.addElevatedOperation("Copy", "@TargetDir@/EZCAP.desktop", "/usr/share/applications/EZCAP.desktop");
	//将桌面文件复制到桌面目录下
            component.addElevatedOperation("Copy", "@TargetDir@/EZCAP.desktop", desktoppath + "/EZCAP.desktop");
        }
	//ubuntu 20.4需要在快捷方式上手动右键点Allow Launching，否则无法双击启动，是否有更好的办法目前未知
	//ubuntu 18 下需要
	// cd ~/Desktop/
	// sudo chown $USER EZCAP.desktop
	// 因为创建的快捷方式归属是root 会被认为是不安全 并且默认没有权限修改
    } catch (e) {
        print(e);
    }
}