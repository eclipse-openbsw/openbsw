[[ $- != *i* ]] && return
export PS1="openbsw> $PS1"

# Bazel shell completion, generated once, persisted via the bazelisk cache mount
_bazelisk_cache_dir="$HOME/.cache/bazelisk"
_bazel_completions="$_bazelisk_cache_dir/bazel-complete.bash"
if [[ ! -s "$_bazel_completions" ]]; then
    # In case completions script does not exist generate completions script via bazelisk
    mkdir -p "$_bazelisk_cache_dir"
    bazelisk completion bash > "$_bazel_completions"
fi
[[ -s "$_bazel_completions" ]] && source "$_bazel_completions"
unset _bazelisk_cache_dir _bazel_completions
