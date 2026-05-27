TEMPLATE = aux

INSTALLER = ezcap_qt_installer

INPUT = $$PWD/config/config.xml $$PWD/packages
ezcap.input = INPUT
ezcap.output = $$INSTALLER
ezcap.commands = /home/mars/Qt/QtIFW-3.2.2/bin/binarycreator -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT}
#ezcap.commands = C:/Qt/QtIFW-4.1.1/bin/binarycreator -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT}
ezcap.CONFIG += target_predeps no_link combine

QMAKE_EXTRA_COMPILERS += ezcap

OTHER_FILES = README
