/* ------------------------------------------------------------
 * SWIG library containing argc and argv multi-argument typemaps
 * ------------------------------------------------------------ */

%typemap(gotype) (int ARGC, char **ARGV) "[]string"

%typemap(in) (int ARGC, char **ARGV) {
  int len = $input.len;
  if (len <= 0 || $input.array == NULL) {
    _swig_gopanic("array must contain at least 1 element");
  }
  $2 = (char **) malloc((len+1)*sizeof(char *));
  if ($2 == NULL) {
    _swig_gopanic("fail allocating memory for array");
  }
  $1 = len;
  {
    int i;
    char *cur = *(char **)($input.array);
    for(i = 0; i < len; i++) {
      $2[i] = cur;
      // Next string start after the current string termination
      cur += strlen(cur) + 1;
    }
    $2[i] = NULL;
  }
}

%typemap(freearg) (int ARGC, char **ARGV) {
  if ($2 != NULL) {
    free((void *)$2);
  }
}
