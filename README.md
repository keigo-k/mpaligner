# What is mpaligner

Mpaligner is the many-to-many string alignment tool based on the generative model
which is modified to find a minimum mapping between two strings, such as notation
and pronunciation. Mpaligner has some functions. The partial annotation function
enables you to give correct alignment to part data by manpower (to provide
Semi-supervised training). The detection function of special data detects data that is
difficult to do alignment (for example [AAA, tri'plei]). The data which alignment
is done is employed as training data. For example, when two strings which
alignment is done are notation and pronunciation, it is employed as training data
to construct a model for grapheme-to-phoneme conversion (g2p conversion).
The license of mpaligner is GNU GPL.


# Publications

Keigo Kubo, Hiromichi Kawanami, Hiroshi Saruwatari, Kiyohiro Shikano, ``Many-to-many Alignment Algorithm for Automatic Pronunciation Annotation on Japanese Unknown Words,'' IPSJ Journal, Vol. 54, No. 2, pp. 452-462, 2013.

Keigo Kubo, Hiromichi Kawanami, Hiroshi Saruwatari, Kiyohiro Shikano, ``Evaluation of Many-to-Many Alignment Algorithm by Automatic Pronunciation Annotation Using Web Text Mining,'' In Proc. INTERSPEECH, Portland, USA, 2012.

Keigo Kubo, Hiromichi Kawanami, Hiroshi Saruwatari, Kiyohiro Shikano, ``Unconstrained Many-to-Many Alignment for Automatic Pronunciation Annotation,'' In Proc. APSIPA, Xi'an, China, 2011.


# Related paper

Y. Okuno, ``Applying mpaligner to machine transliteration with japanese-specific heuristics,'' In Proc. Named Entities Workshop at ACL 2012.


# Install

Go as follow:

```
  $ tar xvfz mpaligner_<version>.tar.gz
  $ cd mpaligner_<version>
  $ make
  $ cp mpaligner <directory included in PATH>
```

# Usage

```
$ cat source/test.utf8.txt | ./script/separate_for_char.pl utf8 \
  source/joint_chars.utf8.txt > source/test.utf8.char_unit
$ mpaligner -i source/test.utf8.char_unit
```

By above commands, mpaligner does alignment over test.utf8.char_unit,
A result of the alignment is output at test.utf8.char_unit.align.
The description of the options is shown below.

```
usage:./mpaligner -i <string> [-o <string>] [-p <string>] [-ai <string>]
           [-ao <string>] [-output_used_mapping <string>] [-s]
           [-hs <int>] [-del] [-ins] [-rx <int>] [-ry <int>]
           [-uc <char>] [-sc <char>] [-jc <char>] [-dic <char>]
           [-training_type 0|1|2|3|4|5] [-kind_of_cityblock 0|1|2]
           [-t <int or float>] [-tfb <int or float>] [-tv <int or float>]
           [-substringCheck] [-noConsecutiveDelAndIns] [-noEqMap]
           [-alignment_type 0|1|2] [-doubtful_context_size <int>]
           [-doubtful_condition 0|1] [-t_nbest <int>] [-f_nbest <int>]
           [-s_nbest <int>] [-output_nbest <int>] [-nbest <int>] [-h]

options:
  -i <string>
    input alignments file.

  -o <string>
    output alignments file. (default <input file name>.align)

  -p <string>
    input previous knowledge file.

  -ai <string>
    input align model file.

  -ao <string>
    output align model file.

  -output_used_mapping <string>
    output used mappings file.

  -s
    print score in output file. (default don't print)

  -hs <int>
    hash size. (default 99,940,009)

  -del
    allow to deletion (del). (default don't allow)

  -ins
    allow to insertion (ins). (default don't allow)

  -dp <float>
    penalty value of del. (default 0.5)

  -ip <float>
    penalty value of ins. (default 1.0)

  -rx <int>
    restrict length of part string X.
    (default 0: 0 is alignment without restrict.)

  -ry <int>
    restrict length of part string Y.
    (default 0: 0 is alignment without restrict.)

  -uc <char>
    unknown char. (default ' ')

  -sc <char>
    separate char. (default '|')

  -jc <char>
    join char. (default ':')

  -dic <char>
    del and ins char. (default '_')

  -training_type 0|1|2|3|4|5
    training type. (defalut 5)
    0 is the forward-backward training that does not introduce city block distance.
    1 is the forward-backward training and the n-best viterbi training, not introducing city block distance respectively.
    2 is the forward-backward training that introduces city block distance.
    3 is the forward-backward training and the n-best viterbi training, introducing city block distance respectively.
    4 is the forward-backward training that prohibits del and ins and the n-best viterbi training within del and ins inference, introducing city block distance respectively. The viterbi training within del and ins inference infers del and ins by using peri-mappings and runs one iteration.
    5 is the forward-backward training that prohibits del and ins, the n-best viterbi training within del and ins inference and the n-best viterbi training, introducing city block distance respectively.

  -t <int or float>
    Threshold for the end of the forward-backward training and the n-best viterbi training. (defalut 0.1)
    If it's more than 1, it's the number of iteration of the training.
    If it's less than 1, it's threshold in change values of a
    parameter by training. If a total of change values of a parameter
    is less than it, the training is end.

  -kind_of_cityblock 0|1|2
    Kind of city block distance.(defalut 0)
    0 uses the number of all characters as city block distance.
    1 uses the number of y's characters as city block distance.
    2 uses the number of x's characters as city block distance.

  -tfb <int or float>
    Threshold for the end of the forward-backward training. (defalut 0.1)

  -tv <int or float>
    Threshold for the end of the n-best viterbi training. (defalut 0.1)

  -substringCheck
    Also check substring in validation of pair data.

  -noConsecutiveDelAndIns
    Don't allow to exist a consecutive del and ins

  -noEqMap
    Not map a x's char size == y's char size mapping.

  -alignment_type 0|1|2
    alignment type. (defalut 1)
    0 is the alignment without join of doubtful mappings.
    1 is the alignment with join of doubtful mappings.
    2 is a detection of pair data that include doubtful mappings.

  -doubtful_context_size <int>
   context size to judge doubtful mappings. (defalut 1)
    This tool regards a mappings that the number of context of front or
    back is small as doubtful mappings. So set the small context size.

  -doubtful_condition 0|1
    condition to judge doubtful mappings. (defalut 1)
    0 regards a mappings that the number of context of front and back
    is small as doubtful mappings. if doubtful_context_size=1 and
    a mapping format is <[context_size]mapping[context_size]>,
    "<[9]mapping[1]><[1]mapping[5]>" is doubtful mappings.

    1 is regards a mappings that the number of context of front or
    back is small as doubtful mappings. If doubtful_context_size=1,
    "<[9]mapping[4]><[1]mapping[5]>" or "<[9]mapping[1]><[3]mapping[5]>"
    is doubtful mappings.

  -t_nbset <int>
    N-best of the n-best viterbi training or a modified n-best viterbi training. (defalut 2)

  -f_nbest <int>
    Internal n-best of first alignment. (defalut 1)
    Alignment type 0's n-best and alignment type 2's n-best follows
    this option. N-best of first alignment in alignment type 1 also follows
    this option.

  -s_nbest <int>
    Internal n-best of second alignment in alignment type 1. (defalut 1)

  -output_nbest <int>
    N-best output. (defalut -f_nbest or -s_nbest)

  -nbest <int>
    A simple option of n-best.
    If alignment_type is 0 or 2, this option is equal to -f_nbest.
    If alignment_type is 1, this option is equal to -s_nbest.
```

# Release information

2012/03/15: version 0.97 Add functions such as partial annotation (Semi-supervised training is now possible.)

2011/01/17: version 0.9 prototype


# Bug report

If you find a bug, please send for it to a following email.

E-mail : keigokubo[at]gmail.com  <- Please replace [at] with @
