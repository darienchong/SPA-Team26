import argparse
import copy
import os
import random
import string


def parse(path, delim, verbose):
    if not path:
        if verbose:
            print('\tNone provided!')
        return []
    if os.path.exists(path):
        arr = []
        with open(path, 'r') as f:
            line = f.readline()
            while line:
                for ele in line.split(delim):
                    arr.append('\"' + ele + '\"')
                line = f.readline()
        if verbose:
            print('\tCompleted parsing of input file!')
        return arr
    else:  # given path does not exist
        raise Exception("Invalid input folder!")


def generate_name(length):
    valid = string.ascii_letters + "0123456789"
    name = ""
    for _ in range(length):
        name += random.choice(valid)
    return name


def generate_iden(num, max_num, length, return_str):
    repeated = dict()
    num_ele = num if num > 0 else max_num
    counter = 0
    arr = []
    while counter < num_ele:
        length = random.randint(1, length)
        iden = generate_name(length)
        if iden in repeated:
            continue
        arr.append(return_str + iden)
        repeated[iden] = 1
        counter += 1
    return arr


def random_sample(lst, ref_lst):
    size = len(lst)
    num_select = random.randint(1, size)
    sample = random.sample(lst, k=num_select)

    ref_lst.extend(sample)
    return sample


def syn_lst_to_declaration(lst, dec):
    return_str = dec
    return_str += ' '
    first = True
    for syn in lst:
        if first:
            return_str += syn
            first = False
        else:
            return_str += ', ' + syn
    return_str += '; '
    return return_str


def attr_ref(weights, lsts, is_name=None):
    weights_copy = copy.deepcopy(weights)
    attr_name = ['.procName', '.varName', '.value', '.stmt#']
    offset = 0
    if is_name is not None:
        if is_name:
            attr_name = attr_name[:2]
            weights_copy = weights_copy[:2]
        else:
            attr_name = attr_name[2:]
            weights_copy = weights_copy[2:]
            offset = 2
    idx = random.choices([i for i in range(len(attr_name))], weights=weights_copy)[0]
    return ' ' + random.choice(lsts[idx + offset]) + attr_name[idx], idx


def select_elem(weights, ent_lst, verbose):
    if verbose:
        print('\t\t\tChoosing syn or attrRef...')
    choice = random.choices([0, 1], weights=weights[0])[0]
    if choice:
        if verbose:
            print('\t\t\tattrRef!')
        return attr_ref(weights[1], ent_lst[choice])[0]
    if verbose:
        print('\t\t\tsyn!')
    return ' ' + random.choice(ent_lst[choice])


def generate_select_result(weights, ent_lst, select_limit, verbose):
    if verbose:
        print('\t\t\tChoosing <> or no <>...')
    bracket_weights = weights[0]
    brackets = random.choices([0, 1], weights=bracket_weights)[0]
    if brackets:
        if verbose:
            print('\t\t\t<>!')
        return_str = ''
        num_ele = random.randint(1, select_limit)  # can consider weighted decaying random choice
        for i in range(num_ele):
            if i == 0:
                return_str += ' <'
                return_str += select_elem(weights[1:], ent_lst, verbose)
            else:
                return_str += ', '
                return_str += select_elem(weights[1:], ent_lst, verbose)
        return_str += '>'
        return return_str
    if verbose:
        print('\t\t\tno <>!')
    return select_elem(weights[1:], ent_lst, verbose)


def generate_clause(generator, weights, lsts, limit, verbose):
    decaying_weights = [0.5, 0.5]
    choice = random.choices([0, 1], weights=decaying_weights)[0]
    counter = 0
    return_str = ''
    while choice and counter < limit:
        if verbose:
            print('\t\t\tand...')
        return_str += ' and'
        return_str += generator(weights, lsts)
        # decaying_weights[1] *= 0.7  # uncomment this line depending if decaying weights are preferred
        choice = random.choices([0, 1], weights=decaying_weights)[0]
        counter += 1
    return return_str


def with_helper(weights, lsts):
    ref_weights = weights[0]
    attr_weights = weights[1]
    choice = random.choices([0, 1, 2], weights=ref_weights)[0]  # TODO: add integer ref
    assert 0 <= choice <= 2, 'Invalid with-cl choice!'
    return_str = ''
    is_name = True
    if choice == 1:
        res, idx = attr_ref(attr_weights, lsts[1])
        return_str += res
        if idx >= 2:
            is_name = False
    elif lsts[choice]:
        if choice != 0:
            is_name = False
        return_str += ' ' + random.choice(lsts[choice])
    else:  # take the syn choice if no iden - iden present only if input files are provided!
        return_str += ' ' + random.choice(lsts[2])  # TODO: add integer ref
        is_name = False
    second_ref = ' ='
    if is_name and lsts[0]:
        temp_weights = [ref_weights[0], ref_weights[1]]
        choice = random.choices([0, 1], weights=temp_weights)[0]
        if choice:
            second_ref += attr_ref(attr_weights, lsts[1], is_name)[0]
        else:
            second_ref += ' ' + random.choice(lsts[0])
    elif is_name:
        second_ref += attr_ref(attr_weights, lsts[1], is_name)[0]
    else:
        temp_weights = [ref_weights[1], ref_weights[2]]  # TODO: add integer ref
        choice = random.choices([1, 2], weights=temp_weights)[0]  # TODO: add integer ref
        if choice == 1:
            second_ref += attr_ref(attr_weights, lsts[1], is_name)[0]
        else:
            second_ref += ' ' + random.choice(lsts[choice])
    return_str += second_ref
    return return_str


def generate_with_clause(weights, lsts, limit, verbose):
    if verbose:
        print('\t\tGenerating with-cl...')
    return_str = ' with'
    return_str += with_helper(weights, lsts)
    return_str += generate_clause(with_helper, weights, lsts, limit, verbose)
    return return_str


def suchthat_helper(weights, lsts):
    suchthat_weights = weights[0]
    transitive_weights = weights[1]
    choice = random.choices([0, 1, 2, 3, 4, 5, 6], weights=suchthat_weights)[0]
    assert 0 <= choice <= 6, 'Invalid suchthat-cl choice!'
    return_str = ''
    first = lsts[choice][0]
    second = lsts[choice][1]
    if choice == 0:  # Modifies
        return_str += ' Modifies('
    elif choice == 1:  # Uses
        return_str += ' Uses('
    else:
        if choice == 2:  # Calls
            return_str += ' Calls'
        elif choice == 3:  # Parent
            return_str += ' Parent'
        elif choice == 4:  # Follows
            return_str += ' Follows'
        elif choice == 5:  # Next
            return_str += ' Next'
        else:  # Affects
            return_str += ' Affects'
        return_str += random.choices(['(', '*('], weights=transitive_weights)[0]
    return_str += random.choice(first)
    return_str += ', '
    return_str += random.choice(second)
    return_str += ')'
    return return_str


def generate_suchthat_clause(weights, lsts, limit, verbose):
    if verbose:
        print('\t\tGenerating suchthat-cl...')
    return_str = ' such that'
    return_str += suchthat_helper(weights, lsts)
    return_str += generate_clause(suchthat_helper, weights, lsts, limit, verbose)
    return return_str


def pattern_helper(weights, lsts):
    ent_lst = lsts[0]
    assign_lst = lsts[1]
    if_lst = lsts[2]
    while_lst = lsts[3]
    choice = random.choices([0, 1, 2], weights=weights)[0]
    assert 0 <= choice <= 2, 'Invalid pattern-cl choice!'
    return_str = ' '
    if choice == 0:  # syn-assign
        if not assign_lst:
            raise Exception('Cannot choose pattern-assign from empty assign syn list!')
        return_str += random.choice(assign_lst)
    elif choice == 1:  # syn-if
        if not if_lst:
            raise Exception('Cannot choose pattern-if from empty if syn list!')
        return_str += random.choice(if_lst)
    else:
        if not while_lst:  # syn-while
            raise Exception('Cannot choose pattern-while from empty while syn list!')
        return_str += random.choice(while_lst)
    return_str += '(' + random.choice(ent_lst)
    if choice != 1:
        return_str += ', _)'
    else:
        return_str += ', _, _)'
    return return_str


def generate_pattern_clause(weights, lsts, limit, verbose):
    if verbose:
        print('\t\tGenerating pattern-cl...')
    return_str = ' pattern'
    return_str += pattern_helper(weights, lsts)
    return_str += generate_clause(pattern_helper, weights, lsts, limit, verbose)
    return return_str


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--num_files', type=int, default=1,
                        help='Number of files to generate. Must be >= 1. Default: 1')
    parser.add_argument('--num_queries', type=int, default=50,
                        help='Number of queries to generate per file. Must be >= 0. Default: 50')
    parser.add_argument('--len_queries', type=int, default=10,
                        help='Max length of each query generated. Must be >= 0. Default: 10')
    ############
    # Synonyms #
    ############
    # Number of synonyms per entity type to generate
    parser.add_argument('--num_syn', type=int, default=10,
                        help='Number of synonyms to generate per entity. Must be >= 1. Default: 10')
    parser.add_argument('--num_proc', type=int, default=-1,
                        help='Number of procedures to generate. Uses num_syn to determine generated amount when '
                             'num_proc < 0. Default: -1')
    parser.add_argument('--num_progl', type=int, default=-1,
                        help='Number of program lines to generate. Uses num_syn to determine generated amount when '
                             'num_progl < 0. Default: -1')
    parser.add_argument('--num_stmt', type=int, default=-1,
                        help='Number of statements to generate. Uses num_syn to determine generated amount when '
                             'num_stmt < 0. Default: -1')
    parser.add_argument('--num_read', type=int, default=-1,
                        help='Number of read statements to generate. Uses num_syn to determine generated amount when '
                             'num_read < 0. Default: -1')
    parser.add_argument('--num_print', type=int, default=-1,
                        help='Number of print statements to generate. Uses num_syn to determine generated amount when '
                             'num_print < 0. Default: -1')
    parser.add_argument('--num_call', type=int, default=-1,
                        help='Number of call statements to generate. Uses num_syn to determine generated amount when '
                             'num_call < 0. Default: -1')
    parser.add_argument('--num_while', type=int, default=-1,
                        help='Number of while statements to generate. Uses num_syn to determine generated amount when '
                             'num_while < 0. Default: -1')
    parser.add_argument('--num_if', type=int, default=-1,
                        help='Number of if statements to generate. Uses num_syn to determine generated amount when '
                             'num_if < 0. Default: -1')
    parser.add_argument('--num_assign', type=int, default=-1,
                        help='Number of assign statements to generate. Uses num_syn to determine generated amount '
                             'when num_assign < 0. Default: -1')
    parser.add_argument('--num_var', type=int, default=-1,
                        help='Number of variables to generate. Uses num_syn to determine generated amount when '
                             'num_var < 0. Default: -1')
    parser.add_argument('--num_const', type=int, default=-1,
                        help='Number of constants to generate. Uses num_syn to determine generated amount when '
                             'num_const < 0. Default: -1')
    ###########
    # Weights #
    ###########
    # Weights of result clause
    parser.add_argument('--weight_result', nargs='+', default=['4', '1'],
                        help='Weight of result clause. Corresponds to frequency of occurrence of tuples and booleans '
                             'respectively in result-cl. Each value must be of int type >= 0. Enter 2 values exactly. '
                             'Default: [4, 1]. Example usage: \'--weight_result 1 0\' to always get tuples.')
    # Weights of tuple
    parser.add_argument('--weight_tuple', nargs='+', default=['1', '3'],
                        help='Weight of tuples. Corresponds to frequency of occurrence of elem and \'<\'elem\'>\' '
                             'respectively in tuples. Each value must be of int type >= 0. Enter 2 values exactly. '
                             'Default: [1, 3]. Example usage: \'--weight_tuple 0 1\' to always get \'<\'elem\'>\'.')
    parser.add_argument('--weight_elem', nargs='+', default=['1', '1'],
                        help='Weight of elem. Corresponds to frequency of occurrence of syn and attrRef respectively '
                             'in elem. Each value must be of int type >= 0. Enter 2 values exactly. Default: [1, '
                             '1]. Example usage: \'--weight_elem 1 0\' to always get syn.')
    # Weights of select clauses
    parser.add_argument('--weight_select', nargs='+', default=['1', '1', '1'],
                        help='Weight of select clause. Corresponds to frequency of occurrence of suchthat-cl, '
                             'with-cl and pattern-cl respectively in select-cl. Each value must be of int type >= 0. '
                             'Enter 3 values exactly. Default: [1, 1, 1] Example usage: \'--weight_select 0 0 1\' to '
                             'always get pattern-cl.')
    # Weights of with clauses
    parser.add_argument('--weight_with', nargs='+', default=['1', '1', '1'],
                        help='Weight of with clause. Corresponds to frequency of occurrence of IDEN, attrRef and syn '
                             'respectively in with-cl. Each value must be of int type >= 0. Enter 3 values exactly. '
                             'Default: [ 1, 1, 1] Example usage: \'--weight_with 0 0 1\' to always get syn with-cl.')
    parser.add_argument('--weight_attr', nargs='+', default=['1', '1', '1', '1'],
                        help='Weight of attrName in attrRef. Corresponds to frequency of occurrence of procName, '
                             'varName, value and stmt# of attrRef respectively in with-cl. Each value must be of int '
                             'type >= 0. Enter 4 values exactly. Default: [1, 1, 1, 1]. Example usage: '
                             '\'--weight_attr 0 0 0 1\' to always get stmt#.')
    # Weights of suchthat clauses
    parser.add_argument('--weight_suchthat', nargs='+', default=['1', '1', '1', '1', '1', '1', '1'],
                        help='Weight of suchthat clause. Corresponds to frequency of  occurrence of ModifiesS/P, '
                             'UsesS/P, Calls(T), Parent(T), Follows(T), Next(T) and Affects(T) respectively in '
                             'suchthat-cl. Each value must be of int type >= 0. Enter 7 values exactly. Default: [1, '
                             '1, 1, 1, 1, 1, 1]. Example usage: \'--weight_suchthat 0 0 0 0 0 0 1\' to always get '
                             'Affects(T) suchthat-cl.')
    parser.add_argument('--weight_transitive', nargs='+', default=['1', '1'],
                        help='Weight of transitive suchthat-cl. Corresponds to the frequency of occurrence of '
                             'non-transitive and transitive clauses respectively in suchthat-cl. Each value must be '
                             'of int type >= 0. Enter 2 values exactly. Default: [1, 1]. Example usage: '
                             '\'--weight_transitive 0 1\' to always get transitive suchthat-cl.')
    # Weights of pattern clauses
    parser.add_argument('--weight_pattern', nargs='+', default=['1', '1', '1'],
                        help='Weight of pattern clause. Corresponds to frequency of occurrence of syn-assign, '
                             'syn-if and syn-while respectively in pattern-cl. Each value must be of int type >= 0. '
                             'Enter 3 values exactly. Default: [1, 1, 1]. Example usage: \'--weight_pattern 0 1 0\' '
                             'to always get syn-if pattern-cl.')
    ################
    # Input/output #
    ################
    # Input files for identified variables, procedures, constants + delimiter to separate inputs in files
    parser.add_argument('--var', type=str, default='',
                        help='[File] containing variable names. Assumes inputs are correct. Default: None')
    parser.add_argument('--proc', type=str, default='',
                        help='[File] containing procedure names. Assumes inputs are correct. Default: None')
    parser.add_argument('--const', type=str, default='',
                        help='[File] containing constant values. Assumes inputs are correct. Default: None')
    parser.add_argument('--delim', type=str, default=' ',
                        help='Delimiter to separate the inputs in files provided. Default: \' \'')
    # Destination folder for generated queries
    parser.add_argument('--start', type=int, default=1, help='File number to start generation from. Default: 1')
    parser.add_argument('--out', type=str, default='queries\\',
                        help='[Path] to the output folder to contain the queries. Will create the directory if it '
                             'does not exist. Default: queries\\')
    ##########
    # Others #
    ##########
    parser.add_argument('--iden_len', type=int, default=10,
                        help='Synonym identity name length. Must be >= 1. Default: 10')
    parser.add_argument('--seed', type=int, default=42, help='RNG seed. Default: 42')
    parser.add_argument('--and_limit', type=int, default=5, help='Number of \'and\' allowed per clause. Default: 5')
    parser.add_argument('--select_limit', type=int, default=5,
                        help='Number of synonyms in select tuple allowed. Must be >= 1. Default: 5')
    parser.add_argument('--timing', type=str, default='5000', help='Timing for each query to run. Default: 5000')
    # Toggle for verbosity - Default: OFF
    parser.add_argument('--verbose', action='store_true',
                        help='Makes the generator verbose for debugging purposes. Default: False')
    # Toggle for erroneous clauses - Default: OFF
    parser.add_argument('--allow_error', action='store_true',
                        help='Allows for clauses with errors to be generated. Default: False')
    parser.add_argument('--comment', nargs='+', default=['NIL'],
                        help='Comment to be generated for test cases together with query id. Default: NIL')
    parser.add_argument('--preset', nargs='+', default=['0', '0'],
                        help='Use a preset to generate queries. First input: 0 default (normal execution), 1 BOOLEAN '
                             'select, 2 tuple select, 3 single select. Second input: 0 default (test iter2 queries), '
                             '1-12 suchthat-cl (1 Follows, 2 FollowsT, 3 Parent, 4 ParentT, 5 Calls, 6 CallsT, '
                             '7 UsesP/S, 8 ModifiesP/S, 9 Next, 10 NextT, 11 Affects, 12 AffectsT), 13-15 pattern-cl '
                             '(13 syn-assign, 14 syn-if, 15 syn-while), 16 with-cl (all variants). Default: [0, '
                             '0]. Example usage: \'--preset 3 11\' to generate single select queries with Affects.')

    args = parser.parse_args()

    # Needed params
    start = args.start
    random.seed(args.seed)
    verbose = args.verbose
    allow_error = args.allow_error
    num_files = args.num_files
    num_queries = args.num_queries
    len_queries = args.len_queries
    num_syn = args.num_syn
    limit = args.and_limit
    select_limit = args.select_limit
    iden_len = args.iden_len
    timing = args.timing
    comment = ' '.join(args.comment)
    preset = list(map(lambda x: int(x), args.preset))
    assert len(preset) == 2, 'Size of preset should be 2'

    # Weights

    result_weights = list(map(lambda x: int(x), args.weight_result))
    assert len(result_weights) == 2, 'Size of result weights should be 2'
    bracket_weights = list(map(lambda x: int(x), args.weight_tuple))
    assert len(bracket_weights) == 2, 'Size of tuple weights should be 2'
    elem_weights = list(map(lambda x: int(x), args.weight_elem))
    assert len(elem_weights) == 2, 'Size of elem weights should be 2'
    select_query_weights = list(map(lambda x: int(x), args.weight_select))
    assert len(select_query_weights) == 3, 'Size of select-cl weights should be 3'

    suchthat_weights = list(map(lambda x: int(x), args.weight_suchthat))
    assert len(suchthat_weights) == 7, 'Size of suchthat-cl weights should be 7'
    weight_transitive = list(map(lambda x: int(x), args.weight_transitive))
    assert len(weight_transitive) == 2, 'Size of transitive weights should be 2'

    # TODO: with_query_weights missing INTEGER ref weights
    attr_name_weights = list(map(lambda x: int(x), args.weight_attr))
    assert len(attr_name_weights) == 4, 'Size of attrName weights should be 4'
    with_weights = list(map(lambda x: int(x), args.weight_with))
    assert len(with_weights) == 3, 'Size of with-cl weights should be 3'

    pattern_query_weights = list(map(lambda x: int(x), args.weight_pattern))
    assert len(pattern_query_weights) == 3, 'Size of pattern-cl weights should be 3'

    # Presets - for ease of use, system testing and to generate queries which more likely are valid
    if preset[0]:  # preset weights to test for iter2 stuff
        num_queries = 20
        len_queries = 2
        num_syn = 5
        iden_len = 2
        limit = 2
        if preset[0] == 1:  # bool select
            result_weights = [0, 1]
            comment = 'BOOLEAN SELECT'
        elif preset[0] == 2:  # tuple select
            result_weights = [1, 0]
            bracket_weights = [0, 1]
            comment = 'tuple SELECT'
        elif preset[0] == 3:
            result_weights = [1, 0]
            bracket_weights = [1, 0]
            comment = 'single SELECT'
        else:
            raise Exception('Invalid input param for preset[0]!')
        if 0 < preset[1] < 13:
            # ModifiesS/P, UsesS/P, Calls(T), Parent(T), Follows(T), Next(T) and Affects(T)
            select_query_weights = [1, 0, 0]
            if preset[1] == 1:  # Follows
                suchthat_weights = [0, 0, 0, 0, 1, 0, 0]
                weight_transitive = [1, 0]
                comment += ' - Follows'
            elif preset[1] == 2:  # FollowsT
                suchthat_weights = [0, 0, 0, 0, 1, 0, 0]
                weight_transitive = [0, 1]
                comment += ' - Follows*'
            elif preset[1] == 3:  # Parent
                suchthat_weights = [0, 0, 0, 1, 0, 0, 0]
                weight_transitive = [1, 0]
                comment += ' - Parent'
            elif preset[1] == 4:  # ParentT
                suchthat_weights = [0, 0, 0, 1, 0, 0, 0]
                weight_transitive = [0, 1]
                comment += ' - Parent*'
            elif preset[1] == 5:  # Calls
                suchthat_weights = [0, 0, 1, 0, 0, 0, 0]
                weight_transitive = [1, 0]
                comment += ' - Calls'
            elif preset[1] == 6:  # CallsT
                suchthat_weights = [0, 0, 1, 0, 0, 0, 0]
                weight_transitive = [0, 1]
                comment += ' - Calls*'
            elif preset[1] == 7:  # Uses
                suchthat_weights = [0, 1, 0, 0, 0, 0, 0]
                comment += ' - UsesP/S'
            elif preset[1] == 8:  # Modifies
                suchthat_weights = [1, 0, 0, 0, 0, 0, 0]
                comment += ' - ModifiesP/S'
            elif preset[1] == 9:  # Next
                suchthat_weights = [0, 0, 0, 0, 0, 1, 0]
                weight_transitive = [1, 0]
                comment += ' - Next'
            elif preset[1] == 10:  # NextT
                suchthat_weights = [0, 0, 0, 0, 0, 1, 0]
                weight_transitive = [0, 1]
                comment += ' - Next*'
            elif preset[1] == 11:  # Affects
                suchthat_weights = [0, 0, 0, 0, 0, 0, 1]
                weight_transitive = [1, 0]
                comment += ' - Affects'
            else:  # AffectsT
                suchthat_weights = [0, 0, 0, 0, 0, 0, 1]
                weight_transitive = [0, 1]
                comment += ' - Affects*'
        elif 13 <= preset[1] <= 15:
            select_query_weights = [0, 0, 1]
            if preset[1] == 13:  # pattern assign
                pattern_query_weights = [1, 0, 0]
                comment += ' - pattern assign'
            elif preset[1] == 14:  # pattern if
                pattern_query_weights = [0, 1, 0]
                comment += ' - pattern if'
            else:  # pattern while
                pattern_query_weights = [0, 0, 1]
                comment += ' - pattern while'
        elif preset[1] == 16:  # with
            select_query_weights = [0, 1, 0]
            comment += ' - with'
        else:
            pattern_query_weights = [0, 1, 1]  # no pattern assign-syn
            suchthat_weights = [1, 1, 1, 0, 0, 1, 1]  # no Parent and Follows
            comment += ' - iter2 queries'

    tuple_weights = [bracket_weights, elem_weights, attr_name_weights]
    suchthat_query_weights = [suchthat_weights, weight_transitive]
    with_query_weights = [with_weights, attr_name_weights]

    # I/O
    if verbose:
        print('Parsing provided input files...')
    var_iden = parse(args.var, args.delim, verbose)
    proc_iden = parse(args.proc, args.delim, verbose)
    const_iden = parse(args.const, args.delim, verbose)
    out_folder = os.path.join(os.path.dirname(os.path.abspath(__file__)), args.out)

    if verbose:
        print('Generating reference entities...')
    procs = generate_iden(args.num_proc, num_syn, iden_len, 'proc')
    prog_l = generate_iden(args.num_progl, num_syn, iden_len, 'prog')
    stmts = generate_iden(args.num_stmt, num_syn, iden_len, 's')
    reads = generate_iden(args.num_read, num_syn, iden_len, 'r')
    prints = generate_iden(args.num_print, num_syn, iden_len, 'prnt')
    calls = generate_iden(args.num_call, num_syn, iden_len, 'call')
    whiles = generate_iden(args.num_while, num_syn, iden_len, 'w')
    ifs = generate_iden(args.num_if, num_syn, iden_len, 'i')
    assigns = generate_iden(args.num_assign, num_syn, iden_len, 'a')
    variables = generate_iden(args.num_var, num_syn, iden_len, 'v')
    consts = generate_iden(args.num_const, num_syn, iden_len, 'cons')

    if verbose:
        print('Checking output folder path: ', out_folder)
    if not os.path.exists(out_folder):
        if verbose:
            print('\tOutput folder path does not exist. Creating folder...')
        os.makedirs(out_folder)

    if verbose:
        print('Generating queries...')
    for i in range(start, num_files + start):
        out_file = args.out + 'queries_' + str(i) + '.txt'
        with open(out_file, 'a') as f:
            if verbose:
                print('Query output file: ', out_file)
            for j in range(1, num_queries + 1):
                f.write(str(j) + ' - ' + comment + '\n')

                # second line: declarations
                ref_lst = []
                syn_proc = random_sample(procs, ref_lst)
                syn_progl = random_sample(prog_l, ref_lst)
                syn_stmt = random_sample(stmts, ref_lst)
                syn_read = random_sample(reads, ref_lst)
                syn_print = random_sample(prints, ref_lst)
                syn_call = random_sample(calls, ref_lst)
                syn_while = random_sample(whiles, ref_lst)
                syn_if = random_sample(ifs, ref_lst)
                syn_assign = random_sample(assigns, ref_lst)
                syn_var = random_sample(variables, ref_lst)
                syn_const = random_sample(consts, ref_lst)

                modifier_user_common = syn_proc + syn_stmt + syn_call + syn_while + syn_if + syn_assign + proc_iden  # TODO: include line ref
                modified_used = syn_var + var_iden + [' _']
                caller_called = syn_proc + proc_iden + ['_']
                parent = syn_stmt + syn_while + syn_if + ['_']  # TODO: include line ref
                child = parent + syn_read + syn_print + syn_call + syn_assign  # TODO: include line ref
                follower_followed = child  # TODO: include line ref
                suchthat_lst = [[modifier_user_common + syn_read, modified_used],
                                [modifier_user_common + syn_print, modified_used], [caller_called, caller_called],
                                [parent, child], [follower_followed, follower_followed],
                                [follower_followed, follower_followed], [follower_followed, follower_followed]]

                with_iden = proc_iden + var_iden + const_iden
                with_proc_name = syn_proc + syn_call
                with_var_name = syn_var + syn_read + syn_print
                with_stmt_num = syn_stmt + syn_read + syn_print + syn_call + syn_while + syn_if + syn_assign
                with_attr_ref = [with_proc_name, with_var_name, syn_const, with_stmt_num]
                with_lst = [with_iden, with_attr_ref, syn_progl]

                tuple_lst = [ref_lst, with_attr_ref]

                pattern_var = syn_var + var_iden + proc_iden if allow_error else syn_var + var_iden
                pattern_lst = [pattern_var, syn_assign, syn_if, syn_while]

                f.write(syn_lst_to_declaration(syn_proc, 'procedure'))
                f.write(syn_lst_to_declaration(syn_progl, 'prog_line'))
                f.write(syn_lst_to_declaration(syn_stmt, 'stmt'))
                f.write(syn_lst_to_declaration(syn_read, 'read'))
                f.write(syn_lst_to_declaration(syn_print, 'print'))
                f.write(syn_lst_to_declaration(syn_call, 'call'))
                f.write(syn_lst_to_declaration(syn_while, 'while'))
                f.write(syn_lst_to_declaration(syn_if, 'if'))
                f.write(syn_lst_to_declaration(syn_assign, 'assign'))
                f.write(syn_lst_to_declaration(syn_var, 'variable'))
                f.write(syn_lst_to_declaration(syn_const, 'constant'))
                f.write('\n')

                # third line: select clauses
                if verbose:
                    print('\tGenerating select result...')
                result_cl = random.choices([0, 1], weights=result_weights)  # result-cl type: 0 for tuple, 1 for boolean
                f.write('Select')
                if result_cl[0] == 0:
                    if verbose:
                        print('\t\tTuple chosen')
                    f.write(generate_select_result(tuple_weights, tuple_lst, select_limit, verbose))
                else:
                    if verbose:
                        print('\t\tBool chosen')
                    f.write(' BOOLEAN')
                query_len = random.randint(0, len_queries)
                query_choices = random.choices([0, 1, 2], weights=select_query_weights, k=query_len)
                # query_choices = [1] * query_len  # for debugging specific select clause
                if verbose:
                    print('\t\tQuery length: ', query_len)
                for choice in query_choices:
                    clause = ''
                    if choice == 0:
                        clause = generate_suchthat_clause(suchthat_query_weights, suchthat_lst, limit, verbose)
                    elif choice == 1:
                        clause = generate_with_clause(with_query_weights, with_lst, limit, verbose)
                    elif choice == 2:
                        clause = generate_pattern_clause(pattern_query_weights, pattern_lst, limit, verbose)
                    f.write(clause)
                f.write('\n')

                # last 2 lines: empty result - to be filled in manually, timing - 5000
                f.write('\n' + timing + '\n')
            if verbose:
                print('Completed generation of queries! Continuing...')


if __name__ == '__main__':
    main()
