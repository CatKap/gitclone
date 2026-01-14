#include "libgit2/include/git2/errors.h"
#include <git2.h>
#include <stdio.h>


/* SSH credentials callback */
int cred_cb(git_cred **out, const char *url, const char *username_from_url,
            unsigned int allowed_types, void *payload) {
    (void)payload; // unused

    return git_cred_ssh_key_from_agent(out, username_from_url);
}

static int transfer_progress_cb(
    const git_indexer_progress *stats,
    void *payload)
{
    fprintf(stderr,
        " Received %u/%u objects (%u/%u deltas), %zu bytes\r",
        stats->received_objects,
        stats->total_objects,
        stats->indexed_deltas,
        stats->total_deltas,
        stats->received_bytes
    );
    return 0;
}

int fast_forward(git_repository *repo)
{
    git_reference *fetch_head = NULL;
    git_object *target = NULL;
    int r;

    /* FETCH_HEAD always points to the last commit */
    r = git_reference_lookup(&fetch_head, repo, "FETCH_HEAD");
    if (r < 0)
        return r;

    r = git_reference_peel(&target, fetch_head, GIT_OBJECT_COMMIT);
    if (r < 0)
        goto cleanup;

    /* reset --hard */
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_opts.checkout_strategy =
        GIT_CHECKOUT_FORCE | GIT_CHECKOUT_RECREATE_MISSING;

    r = git_checkout_tree(repo, target, &checkout_opts);
    if (r < 0)
        goto cleanup;

    r = git_repository_set_head_detached(repo, git_object_id(target));

cleanup:
    git_object_free(target);
    git_reference_free(fetch_head);
    return r;
}

int main(int argc, char **argv) {
    git_libgit2_init();
    if(argc < 3){
      printf("Usage: gitclone [url] [destination dir] {optional:branch}\n");
    return 1;
    }

    const char *branch = (argc >= 4) ? argv[3] : NULL;
    git_repository *repo = NULL;
    git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    fetch_opts.callbacks.transfer_progress = transfer_progress_cb;

    /* shallow clone */
    fetch_opts.depth = 1;
fetch_opts.callbacks.transfer_progress = transfer_progress_cb;
    fetch_opts.callbacks.credentials = cred_cb;
    opts.fetch_opts = fetch_opts;

    if (branch) {
        printf("Cloning branch %s\n", branch);
        opts.checkout_branch = branch;
    }

    const char* url = argv[1];
    const char* directory = argv[2];
    
    int r = git_clone(&repo, url, directory, &opts);
    if(r == GIT_EEXISTS){
      fprintf(stderr, "Repo exsists, trying to pull...\n");
      
      int error = git_repository_open(&repo, directory);
      if (error < 0) {
        const git_error *e = git_error_last();
        printf("Error %d/%d: %s\n", error, e->klass, e->message);
        exit(error);
      }
      printf("FF\n"); 
      git_remote *remote = NULL;
      git_remote_lookup(&remote, repo, "origin");
      r = git_remote_fetch(remote, NULL, &fetch_opts, NULL);
      r = fast_forward(repo);
    }

    if (r < 0) {

        const git_error *e = git_error_last();
        fprintf(stderr, "Error: %s\n", e && e->message ? e->message : "unknown\n");
    }

    git_repository_free(repo);
    git_libgit2_shutdown();
    return r;
}
