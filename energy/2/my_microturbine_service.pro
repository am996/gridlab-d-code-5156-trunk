# This file generated by staff_codegen
# For more information please visit: http://code.google.com/p/staff/

QT          += core
QT          -= gui

TARGET       = my_microturbine_service
CONFIG      += console
CONFIG      -= app_bundle

TEMPLATE     = lib

COMPONENT    = my_microturbine

STAFF_HOME   = $$(STAFF_HOME)

!isEmpty (STAFF_HOME) {
  INCLUDEPATH    += $$STAFF_HOME/include
  LIBS           += -L$$STAFF_HOME/lib
  STAFF_CODEGEN   = $$STAFF_HOME/bin/staff_codegen
  INSTALL_PATH    = $$STAFF_HOME/components/$$COMPONENT
} else {
  STAFF_CODEGEN   = staff_codegen
  INSTALL_PATH    = /usr/lib/staff/components/$$COMPONENT
}


LIBS        += -lstaffutils -lstaffxml -lstaffcommon -lstaffcomponent


STAFF_INTERFACES = \
    src/my_microturbine.h    \


SOURCES     += \
    src/my_microturbineImpl.cpp    \


HEADERS     += \
    src/my_microturbine.h \
    src/my_microturbineImpl.h    \


staff_service_wrapper_decl.name = staff service wrapper header
staff_service_wrapper_decl.input = STAFF_INTERFACES
staff_service_wrapper_decl.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}Wrapper.h
staff_service_wrapper_decl.commands = $$STAFF_CODEGEN -tcomponent -c${QMAKE_FILE_PATH} ${QMAKE_FILE_BASE}.h
staff_service_wrapper_decl.variable_out = GENERATED_FILES
QMAKE_EXTRA_COMPILERS += staff_service_wrapper_decl

staff_service_wrapper_impl.name = staff service wrapper wrapper implementation
staff_service_wrapper_impl.input = STAFF_INTERFACES
staff_service_wrapper_impl.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}Wrapper.cpp
staff_service_wrapper_impl.depends = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}Wrapper.h ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.h
staff_service_wrapper_impl.commands = $$escape_expand("\\n")
staff_service_wrapper_impl.variable_out = GENERATED_SOURCES
QMAKE_EXTRA_COMPILERS += staff_service_wrapper_impl

staff_component_decl.name = staff service wrapper header
staff_component_decl.input = STAFF_INTERFACES
staff_component_decl.output = ${QMAKE_FILE_PATH}/ComponentImpl.h
staff_component_decl.commands = $$escape_expand("\\n")
staff_component_decl.variable_out = GENERATED_FILES
QMAKE_EXTRA_COMPILERS += staff_component_decl

staff_component_impl.name = staff service wrapper wrapper implementation
staff_component_impl.input = STAFF_INTERFACES
staff_component_impl.output = ${QMAKE_FILE_PATH}/ComponentImpl.cpp
staff_component_impl.depends = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.h ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}Wrapper.h ${QMAKE_FILE_PATH}/ComponentImpl.h
staff_component_impl.commands = $$escape_expand("\\n")
staff_component_impl.variable_out = GENERATED_SOURCES
QMAKE_EXTRA_COMPILERS += staff_component_impl

target.path = $$INSTALL_PATH

wsdls.files = \
wsdls.path = $$INSTALL_PATH

INSTALLS += target wsdls


