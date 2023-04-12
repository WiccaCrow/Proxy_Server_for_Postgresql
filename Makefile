.PHONY: clean fclean re all

NAME = TCPserv

###################################################################################
#                               Compiler & Flags                                  #
###################################################################################

# CXX       =   g++
# CPPLIB	=
CXX       =   gcc
CPPFLAGS  =   -Wall -Wextra -Werror
CPPLIB    =   -lstdc++
COMP_CONST =  -D LOGS_DIR=\"${LOGS_DIR}\"

ifeq ($(DEBUG), 1)
	CPPFLAGS += -g3
endif

###################################################################################
#                              Directories & Files                                #
###################################################################################

SRCS_DIR     = src
OBJS_DIR     = .obj
DEPS_DIR     = .deps
TMP_DIR      = .tmp
INCLUDE_DIR  = include
LOGS_DIR     = logs
DFLT_DIR     = default

SRCS     =  main.cpp		Server.cpp \

OBJS = $(addprefix $(OBJS_DIR)/, $(SRCS:.cpp=.o))
DEPS = $(addprefix $(DEPS_DIR)/, $(SRCS:.cpp=.d))

###################################################################################
#                                   Commands                                      #
###################################################################################

comp: makedir $(NAME)

all: 
	$(MAKE) comp DEBUG=1

makedir:
	@if ! [ -d ${OBJS_DIR} ] ; then mkdir ${OBJS_DIR} ; fi
	@if ! [ -d ${DEPS_DIR} ] ; then mkdir ${DEPS_DIR} ; fi
	@if ! [ -d ${LOGS_DIR} ] ; then mkdir ${LOGS_DIR} ; fi
	@if ! [ -d ${TMP_DIR} ] ; then mkdir ${TMP_DIR} ; fi


$(NAME): $(OBJS)
	$(CXX) $(CPPFLAGS) $^ -o $@ $(CPPLIB) -I $(INCLUDE_DIR)

-include $(DEPS)
$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@ $(CPPLIB) \
        -I $(INCLUDE_DIR) \
        ${COMP_CONST} \
        -MMD -MF $(patsubst ${OBJS_DIR}/%.o,${DEPS_DIR}/%.d,$@) \
		

clean:
	rm -rf ${DEPS_DIR} ${OBJS_DIR} ${LOGS_DIR} ${TMP_DIR}

fclean: clean
	rm -rf $(NAME)

re: fclean all

###################################################################################
#                                    Tests                                        #
###################################################################################


