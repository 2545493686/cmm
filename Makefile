BUILD_DIR = ./build
TOOLS_DIR = ./tools
INCLUDE_DIR = ./include
BUILD_SRC_DIR = $(BUILD_DIR)/src
BUILD_LIB_DIR = $(BUILD_DIR)/lib

LIB_SC = ./lib/sc
LIB_SC_MAP = $(LIB_SC)/map

TEST_DIR = ./tests
TEST_BUILD_DIR = $(TEST_DIR)/build

flex:
	@mkdir -p $(BUILD_SRC_DIR)
	@flex --outfile $(BUILD_SRC_DIR)/lex.yy.c cmm_flex.l
	@gcc $(BUILD_SRC_DIR)/lex.yy.c -g -o $(BUILD_DIR)/cmm_flex -I$(INCLUDE_DIR)

flex_run: 
	@make clean
	@make flex
	@python3 $(TOOLS_DIR)/input_generater.py >> $(BUILD_DIR)/flex_input.txt
	@$(BUILD_DIR)/cmm_flex < $(BUILD_DIR)/flex_input.txt >> $(BUILD_DIR)/flex_output.txt
	@echo << "flex output:"
	@cat < $(BUILD_DIR)/flex_output.txt
	@echo << "\n"
	@echo << "token count:"
	@python3 $(TOOLS_DIR)/get_tokens_count.py < $(BUILD_DIR)/flex_output.txt
	@echo << "\n"

# 将 cmm_syntax_tree.c 编译为 .so
syntax_tree:
	@mkdir -p $(BUILD_LIB_DIR)
	@gcc -fPIC -shared -o $(BUILD_LIB_DIR)/libsyntax_tree.so cmm_syntax_tree.c -I$(INCLUDE_DIR)

parser:
	@make syntax_tree
	@mkdir -p $(BUILD_SRC_DIR)
	@gcc cmm_parser.c -Wall -Werror -g -Og -o $(BUILD_DIR)/cmm_parser -I$(INCLUDE_DIR)\
	 -L$(BUILD_LIB_DIR) -lsyntax_tree -Wl,-rpath=$(BUILD_LIB_DIR)
	@echo "parser build success"

parser_run:
	@make clean
	@make flex
	@make parser
	@python3 tools/input_generater.py | ./build/cmm_flex | ./build/cmm_parser

syntax_tree_test:
	@make clean
	@make flex
	@make parser
	@mkdir -p $(TEST_BUILD_DIR)/syntax_tree_test
	@gcc $(TEST_DIR)/syntax_tree_test.c -Wall -Werror -g -Og -o\
	  $(TEST_BUILD_DIR)/syntax_tree_test/syntax_tree_test -I$(INCLUDE_DIR)\
	 -L$(BUILD_LIB_DIR) -lsyntax_tree -Wl,-rpath=$(BUILD_LIB_DIR)
	@python3 tools/input_generater.py | ./build/cmm_flex | ./build/cmm_parser -s=debug\
		>> $(TEST_BUILD_DIR)/syntax_tree_test/origin_output.txt
	@python3 tools/input_generater.py | ./build/cmm_flex | ./build/cmm_parser  \
		>> $(TEST_BUILD_DIR)/syntax_tree_test/interim_output.txt
	@echo "interim_output.txt\n"
	@cat $(TEST_BUILD_DIR)/syntax_tree_test/interim_output.txt
	@echo "\n"
	@cat $(TEST_BUILD_DIR)/syntax_tree_test/interim_output.txt |	$(TEST_BUILD_DIR)/syntax_tree_test/syntax_tree_test \
		>> $(TEST_BUILD_DIR)/syntax_tree_test/target_output.txt
	@echo "target_output.txt\n"
	@cat $(TEST_BUILD_DIR)/syntax_tree_test/target_output.txt
	@echo "\n"
	@echo "diff:"
	@diff $(TEST_BUILD_DIR)/syntax_tree_test/origin_output.txt $(TEST_BUILD_DIR)/syntax_tree_test/target_output.txt

runtime:
	@make syntax_tree
	@mkdir -p $(BUILD_SRC_DIR)
	@gcc cmm_runtime.c -Wall -Werror -g -Og -o $(BUILD_DIR)/cmm_runtime -I$(INCLUDE_DIR)\
	 -L$(BUILD_LIB_DIR) -lsyntax_tree -Wl,-rpath=$(BUILD_LIB_DIR)\
	 -L$(LIB_SC_MAP) -lsc_map  -Wl,-rpath=$(LIB_SC_MAP)
	@echo "runtime build success"

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(TEST_BUILD_DIR)
.PHONY: clean