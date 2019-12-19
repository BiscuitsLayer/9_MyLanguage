#include "Frontend.h"

Elem_t *Tokens::Tokenization (FILE *readfile) {
    struct stat file_info = {};
    stat (INPUTFILE, &file_info);

    char *program = (char *) calloc (file_info.st_size + EXTRA_BUF_SIZE, sizeof (char));
    program[file_info.st_size] = EOF;
    fread (program, sizeof (char), file_info.st_size, readfile);

    Elem_t *tokens = nullptr;
    tokens = (Elem_t *) calloc (file_info.st_size, sizeof(Elem_t));

    char *s = program;
    bool line_inc = false; //Надо ли увеличить счётчик строк после данного токена
    size_t line_num = 1; //Счётчик строк
    size_t pass = 1;
    while (*s != EOF) {
        (tokens[idx].name) = s;
        while (!isspace(*s) && *s != EOF) {
            ++s;
        }
        if (*s == EOF) {
            *s = '\0';
            break;
        } else {
            if (*s == '\n')
                line_inc = true;
            *s = '\0';
        }
        ++s;
        if (strcmp (tokens[idx].name, "") != 0 && strcmp (tokens[idx].name, "\0") != 0 && strcmp (tokens[idx].name, "\t") != 0) {
            tokens[idx].line_num = line_num;
            if (line_inc) {
                ++line_num;
                line_inc = false;
            }
            Tokens::TokenHandle (tokens, pass); //Предварительная обработка токена
            ++idx; //Переход к следующему токену
        }
    }
    idx = 0;
    ++pass; //Двухпроходная компиляция
    while (tokens[idx].name) {
        Tokens::TokenHandle (tokens, pass); //Постобработка токена
        ++idx; //Переход к следующему токену
    }
    idx = 0;
    return tokens;
}

void Tokens::TokenHandle (Elem_t *tokens, size_t pass) {
    //Счётчик (колво "{" - колво "}"), если == 0 то мы не в функции, => объявляем глобальные переменные
    if (strcmp (tokens[idx].name, LangCommands[OPEN_BRACE]) == 0) {
        ++brace_flag;
        just_entered_function = false;
    }
    if (strcmp (tokens[idx].name, LangCommands[CLOSE_BRACE]) == 0) {
        --brace_flag;
        just_entered_function = false;
    }
    if (brace_flag == 0 && !just_entered_function)
        function_flag = -1;

    size_t size = sizeof (LangCommands) / sizeof (char *);
    for (size_t i = 0; i < size; ++i) {
        if (strcmp(tokens[idx].name, LangCommands[i]) == 0) {
            tokens[idx].type = TYPE_SYS;
            return;
        }
    }
    size = sizeof (Operations) / sizeof (char *);
    for (size_t i = 0; i < size; ++i) {
        if (strcmp(tokens[idx].name, Operations[i]) == 0) {
            tokens[idx].type = TYPE_OP;
            return;
        }
    }
    if (isdigit(tokens[idx].name[0])) {
        tokens[idx].type = TYPE_NUM;
        return;
    }
    //Стратегический ход: сначала прописан кейс для функций, только в том случае, когда этот кейс не пройден, он проверит переменная ли это
    else if (idx > 0 && strcmp(tokens[idx - 1].name, LangCommands[FUNCTION]) == 0) {
        tokens[idx].type = TYPE_FUNC;
        function_flag = Tree::FuncSearch (tokens[idx].name, true);
        just_entered_function = true;
        return;
    }
    else if (idx > 0 && strcmp(tokens[idx - 1].name, LangCommands[CALL]) == 0) {
        tokens[idx].type = TYPE_FUNC;
        return;
    }
    //Для функций обязательно чтобы предыдущий токен был CALL или FUNCTION, для переменных - нет
    else if ((idx > 0 && strcmp(tokens[idx - 1].name, LangCommands[VAR]) == 0) || (Tree::VarSearch (tokens[idx].name) != NOTFOUND && tokens[idx].line_num >= vars[Tree::VarSearch (tokens[idx].name)].line_num) || just_entered_function) {
        tokens[idx].type = TYPE_VAR;
        size_t temp = Tree::VarSearch (tokens[idx].name, true);
        if (just_added_variable) { //Если только что добавили переменную
            vars[temp].val = function_flag;
            vars[temp].line_num = tokens[idx].line_num;
            just_added_variable = false;
        }
        return;
    }
    else if (pass == 2) {
        printf ("Error! Unknown token \"%s\" in line %d\n", tokens[idx].name, tokens[idx].line_num);
        //exit (1);
    }
}

Node *RD::GetG (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *ans = node;
    node->data = SEMICOLON;
    node->type = TYPE_SYS;
    assert (strcmp (tokens[idx].name, LangCommands[VAR]) == 0 || strcmp (tokens[idx].name, LangCommands[FUNCTION]) == 0 );

    while (tokens[idx].name && ((strcmp (tokens[idx].name, LangCommands[VAR]) == 0) || strcmp (tokens[idx].name, LangCommands[FUNCTION]) == 0)) {
        if (strcmp(tokens[idx].name, LangCommands[VAR]) == 0) {
            ++idx;
            node->left = Tree::NodeInit();
            node->left = RD::GetAs(tokens);
            node->right = Tree::NodeInit();
            node->right->data = SEMICOLON;
            node->right->type = TYPE_SYS;
            node = node->right;
        }
        else if (strcmp(tokens[idx].name, LangCommands[FUNCTION]) == 0) {
            node->left = Tree::NodeInit();
            node->left = RD::GetF(tokens);
            node->right = Tree::NodeInit();
            node->right->data = SEMICOLON;
            node->right->type = TYPE_SYS;
            node = node->right;
        }
    }
    Tree::TreeOffsetCorrecter (ans);
    while (flag) {
        flag = false;
        Tree::EmptyNodesCleaner(ans);
    }

    flag = true;
    idx = 0;
    if (main_flag < 0) {
        printf ("Error! No main function found\n");
        exit (1);
    }
    return ans;
}

Node *RD::GetF (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *base = node;
    node->type = TYPE_FUNC;
    assert (strcmp (tokens[idx].name, LangCommands[FUNCTION]) == 0);
    ++idx;
    node->data = Tree::FuncSearch (tokens[idx].name);
    if (strcmp (tokens[idx].name, "main") == 0)
        main_flag = node->data;
    function_flag = node->data;
    ++idx;

    node->left = Tree::NodeInit ();
    node->left = RD::GetFArgs (tokens);


    assert (strcmp (tokens[idx].name, LangCommands[OPEN_BRACE]) == 0);
    ++idx;

    while (strcmp (tokens[idx].name, LangCommands[CLOSE_BRACE]) != 0) {
        node->right = Tree::NodeInit ();
        node->right = RD::GetOp(tokens);
        node = node->right;
    }
    ++idx;
    function_flag = -1;
    return base;
}

Node *RD::GetFArgs (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *ans = node;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    if (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0) {
        ++idx;
        return nullptr;
    }

    node->left = Tree::NodeInit ();
    node->left->type = TYPE_SYS;
    node->left->data = COMMA;
    node = node->left;

    size_t var_count = 0;

    while (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) != 0) {
        node->right = Tree::NodeInit ();
        assert (isalpha(tokens[idx].name[0]));
        node->right = RD::GetID(tokens);
        ++var_count;
        if (strcmp(tokens[idx].name, LangCommands[COMMA]) == 0) {
            node->left = Tree::NodeInit();
            node->left->type = TYPE_SYS;
            node->left->data = COMMA;
            node = node->left;
            ++idx;
        }
    }
    funcs[function_flag].val = var_count;
    ++idx;
    return ans;
}

Node *RD::GetAs (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *ans = node;
    node->left = RD::GetID (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[EQUAL]) == 0);
    node->data = EQUAL;
    node->type = TYPE_SYS;
    ++idx;
    if (strcmp (tokens[idx].name, LangCommands[CALL]) == 0)
        node->right = RD::GetFCall (tokens);
    else
        node->right = RD::GetE (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[SEMICOLON]) == 0);
    ++idx;
    return ans;
}

Node *RD::GetID (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    if (tokens[idx].type == TYPE_VAR) {
        node->type = TYPE_VAR;
        node->data = Tree::VarSearch (tokens[idx].name);
        if (node->data == NOTFOUND) {
            printf ("Error! Variable \"%s\" not found", tokens[idx].name);
            exit (4);
        }
    }
    else {
        printf ("Error! Unknown token \"%s\" in line %d\n", tokens[idx].name, tokens[idx].line_num);
        exit (1);
    }
    ++idx;
    return node;
}

Node *RD::GetE (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    node->left = RD::GetT (tokens);
    while (strcmp (tokens[idx].name, Operations[OP_SUM]) == 0 || strcmp (tokens[idx].name, Operations[OP_SUB]) == 0) {
        node->type = TYPE_OP;
        char op = tokens[idx].name[0];
        ++idx;
        if (op == '+')
            node->data = OP_SUM;
        else
            node->data = OP_SUB;
        node->right = RD::GetT (tokens);
        if (strcmp (tokens[idx].name, Operations[OP_SUM]) == 0|| strcmp (tokens[idx].name, Operations[OP_SUB]) == 0) {
            Node *new_node = Tree::NodeInit (nullptr, node);
            node = new_node;
        }
    }
    if (node->right)
        return node;
    else
        return node->left;
}

Node *RD::GetT (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    node->left = RD::GetS (tokens);
    while (strcmp (tokens[idx].name, Operations[OP_MUL]) == 0 || strcmp (tokens[idx].name, Operations[OP_DIV]) == 0) {
        node->type = TYPE_OP;
        char op = tokens[idx].name[0];
        ++idx;
        if (op == '*')
            node->data = OP_MUL;
        else
            node->data = OP_DIV;
        node->right = RD::GetS (tokens);
        if (strcmp (tokens[idx].name, Operations[OP_MUL]) == 0 || strcmp (tokens[idx].name, Operations[OP_DIV]) == 0) {
            Node *new_node = Tree::NodeInit (nullptr, node);
            node = new_node;
        }
    }
    if (node->right)
        return node;
    else
        return node->left;
}

Node *RD::GetS (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    node->left = GetP (tokens);
    if (tokens[idx].name[0] == '^') {
        node->type = TYPE_OP;
        node->data = OP_POW;
        ++idx;
        node->right = GetP (tokens);
    }
    if (node->right)
        return node;
    else
        return node->left;
}

Node *RD::GetP (Elem_t *tokens) {
    Node *node = nullptr;
    if (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0) {
        ++idx;
        node = RD::GetE (tokens);
        assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
        ++idx;
    }
    else if (isdigit (tokens[idx].name[0])) {
        node = RD::GetN(tokens);
    }
    else if (strcmp (tokens[idx].name, Operations[OP_DERIV]) == 0) {
        node = RD::GetDeriv (tokens);
    }
    else if (isalpha (tokens[idx].name[0])) {
        node = RD::GetID (tokens);
    }
    else {
        printf ("Error! Unknown token \"%s\" in line %d\n", tokens[idx].name, tokens[idx].line_num);
        exit (1);
    }
    return node;
}

Node *RD::GetN (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    for (size_t i = 0; i < strlen(tokens[idx].name); ++i)
        assert (isdigit (tokens[idx].name[i]) || tokens[idx].name[i] == '.');
    node->data = strtod (tokens[idx].name, nullptr);
    node->type = TYPE_NUM;
    ++idx;
    return node;
}

Node *RD::GetOp (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    node->type = TYPE_SYS;
    node->data = OP;
    node->left = Tree::NodeInit ();

    if (strcmp (tokens[idx + 1].name, LangCommands[EQUAL]) == 0) {
        node->left = RD::GetAs (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[IF]) == 0) {
        node->left = RD::GetIf (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[WHILE]) == 0) {
        node->left = RD::GetWhile (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[RET]) == 0) {
        node->left = RD::GetReturn (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[PUT]) == 0) {
        node->left = RD::GetPut (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[GET]) == 0) {
        node->left = RD::GetGet (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[OPEN_BRACE]) == 0) {
        Node *base = node;
        ++idx;
        while (strcmp (tokens[idx].name, LangCommands[CLOSE_BRACE]) != 0) {
            node->right = RD::GetOp (tokens);
            node->right->right = Tree::NodeInit();
            node = node->right;
        }
        node = base;
        ++idx;
    }
    else if (strcmp (tokens[idx].name, LangCommands[VAR]) == 0) {
        ++idx;
        node->left =  RD::GetAs (tokens);
    }
    else {
        printf ("Error! Unknown operator \"%s\" in line %d\n", tokens[idx].name, tokens[idx].line_num);
        exit (1);
    }
    return node;
}

Node *RD::GetReturn (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, LangCommands[RET]) == 0);
    ++idx;
    node->data = RET;
    node->type = TYPE_SYS;
    if (strcmp (tokens[idx].name, LangCommands[CALL]) == 0)
        node->left = RD::GetFCall (tokens);
    else
        node->left = RD::GetE (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[SEMICOLON]) == 0);
    ++idx;
    return node;
}

Node *RD::GetDeriv (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, Operations[OP_DERIV]) == 0);
    node->type = TYPE_OP;
    node->data = OP_DERIV;
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    node->right = RD::GetE (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[COMMA]) == 0);
    ++idx;
    assert (tokens[idx].type == TYPE_VAR);
    node->left = RD::GetID (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
    ++idx;
    return node;
}

Node *RD::GetFCall (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *base = node;
    node->type = TYPE_FUNC;
    assert (strcmp (tokens[idx].name, LangCommands[CALL]) == 0);
    ++idx;
    node->data = Tree::FuncSearch (tokens[idx].name);
    ++idx;
    node->left = Tree::NodeInit ();
    node->left = RD::GetFCallArgs (tokens, funcs[(int)node->data].val);
    return base;
}

Node *RD::GetFCallArgs (Elem_t *tokens, size_t var_count) {
    Node *node = Tree::NodeInit ();
    Node *ans = node;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    if (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0) {
        ++idx;
        return nullptr;
    }

    node->left = Tree::NodeInit ();
    node->left->type = TYPE_SYS;
    node->left->data = COMMA;
    node = node->left;

    size_t var_count_check = 0;

    while (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) != 0) {
        node->right = RD::GetE (tokens);
        ++var_count_check;
        if (strcmp (tokens[idx].name, LangCommands[COMMA]) == 0) {
            node->left = Tree::NodeInit();
            node->left->type = TYPE_SYS;
            node->left->data = COMMA;
            node = node->left;
            ++idx;
        }
    }
    ++idx;
    return ans;
}

Node *RD::GetIf (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, LangCommands[IF]) == 0);
    node->data = IF;
    node->type = TYPE_SYS;
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    node->left = Tree::NodeInit ();
    node->left = RD::GetC (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
    ++idx;
    node->right = Tree::NodeInit ();
    node->right->data = IF_ELSE;
    node->right->type = TYPE_SYS;
    node->right->left = Tree::NodeInit ();
    node->right->left = RD::GetOp (tokens);
    if (strcmp (tokens[idx].name, LangCommands[IF_ELSE]) == 0) {
        ++idx;
        node->right->right = Tree::NodeInit ();
        node->right->right = RD::GetOp (tokens);
    }
    return node;
}

Node *RD::GetWhile (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, LangCommands[WHILE]) == 0);
    node->data = WHILE;
    node->type = TYPE_SYS;
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    node->left = Tree::NodeInit ();
    node->left = RD::GetC (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
    ++idx;
    node->right = Tree::NodeInit ();
    node->right = RD::GetOp (tokens);
    return node;
}

Node *RD::GetC (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    node->left = Tree::NodeInit ();
    node->left = GetE (tokens);
    for (size_t i = OP_ABOVE; i <= OP_UNEQUAL; ++i) {
        if (strcmp (tokens[idx].name, Operations[i]) == 0)
            node->data = i;
    }
    node->type = TYPE_OP;
    if (node->data == 0) {
        printf ("Error! Wrong condition in line %d\n", tokens[idx].line_num);
        exit (3);
    }
    ++idx;
    node->right = Tree::NodeInit ();
    node->right = GetE (tokens);
    if (node->right)
        return node;
    else
        return node->left;
}

Node *RD::GetPut (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, LangCommands[PUT]) == 0);
    node->data = PUT;
    node->type = TYPE_SYS;
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    node->left = Tree::NodeInit ();
    if (strcmp (tokens[idx].name, LangCommands[CALL]) == 0)
        node->left = RD::GetFCall (tokens);
    else
        node->left = RD::GetE (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[SEMICOLON]) == 0);
    ++idx;
    return node;
}

Node *RD::GetGet (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, LangCommands[GET]) == 0);
    node->data = GET;
    node->type = TYPE_SYS;
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    node->left = Tree::NodeInit ();
    node->left = RD::GetID (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[SEMICOLON]) == 0);
    ++idx;
    return node;
}