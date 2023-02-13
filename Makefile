BUILD_DIR = ./build
TOOLS_DIR = ./tools
INCLUDE_DIR = ./include
BUILD_SRC_DIR = $(BUILD_DIR)/src

flex:
	@mkdir -p $(BUILD_SRC_DIR)
	@flex --outfile $(BUILD_SRC_DIR)/lex.yy.c cmm_flex.l
	@gcc $(BUILD_SRC_DIR)/lex.yy.c -o $(BUILD_DIR)/cmm_flex -I$(INCLUDE_DIR)

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

parser:
	@mkdir -p $(BUILD_SRC_DIR)
	@gcc cmm_parser.c -Wall -Werror -o $(BUILD_DIR)/cmm_parser -I$(INCLUDE_DIR) -L -lsc_array
	@echo "parser build success"

parser_run:
	@make clean
	@make flex
	@make parser
	@python3 $(TOOLS_DIR)/input_generater.py >> $(BUILD_DIR)/flex_input.txt
	@$(BUILD_DIR)/cmm_flex < $(BUILD_DIR)/flex_input.txt >> $(BUILD_DIR)/flex_output.txt
	@$(BUILD_DIR)/cmm_parser < $(BUILD_DIR)/flex_output.txt >> $(BUILD_DIR)/parser_output.txt
	@echo << "parser output:"
	@cat < $(BUILD_DIR)/parser_output.txt
	@echo << "\n"

clean:
	@rm -rf ./build
.PHONY: clean