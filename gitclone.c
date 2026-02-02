#include "libgit2/include/git2/errors.h"
#include <git2.h>
#include <stdio.h>
#include <string.h>

char* PAT = NULL;
char* SSH_KEY_FILE = NULL;
char* SSH_PUB_FILE = NULL;

// Callback for GitHub PAT via HTTPS
int github_pat_cb(git_cred **out, const char *url, const char *username,
                  unsigned int allowed_types, void *payload) {
    
    
    if (!PAT) {
        fprintf(stderr, "No pat token provieded!\n");
        return GIT_EUSER;
    }
    
    if (git_cred_userpass_plaintext_new(out, "x-access-token", PAT) == 0) {
        return 0;
    }
    
    if (git_cred_userpass_plaintext_new(out, "oauth2", PAT) == 0) {
        return 0;
    }
    
    return git_cred_userpass_plaintext_new(out, "token", PAT);
}


/* SSH credentials callback */
int cred_agent_cb(git_cred **out, const char *url, const char *username_from_url,
            unsigned int allowed_types, void *payload) {
    (void)payload; // unused

    return git_cred_ssh_key_from_agent(out, username_from_url);
}


int cred_acquire_cb(
    git_credential **out,
    const char *url,
    const char *username_from_url,
    unsigned int allowed_types,
    void *payload)
{
    const char *username = username_from_url ? username_from_url : "git";
    return git_credential_ssh_key_new(
        out,
        username,
        SSH_PUB_FILE,   // public key (can be NULL)
        SSH_KEY_FILE,       // private key
        NULL             // or NULL if none
    );
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

int fast_forward(git_repository *repo, const char *branch_name)
{
    git_reference *remote_ref = NULL;
    git_reference *local_ref = NULL;
    git_object *target = NULL;
    int r = -1;
    char refname[256];
    
    if(NULL != branch_name){

      snprintf(refname, sizeof(refname), "refs/remotes/origin/%s", branch_name);
      r = git_reference_lookup(&remote_ref, repo, refname);
    }

    if (r < 0) {
        /* If not find remote branch */
        r = git_reference_lookup(&remote_ref, repo, "FETCH_HEAD");
        if (r < 0)
            return r;
    }
    
    /* Get commit ref */
    r = git_reference_peel(&target, remote_ref, GIT_OBJECT_COMMIT);
    if (r < 0)
        goto cleanup;
    
    /* Opts for force backoff */
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_opts.checkout_strategy =
        GIT_CHECKOUT_FORCE | GIT_CHECKOUT_RECREATE_MISSING;
    
    r = git_checkout_tree(repo, target, &checkout_opts);
    if (r < 0)
        goto cleanup;
    
    /* Update local branch to this commit  */
    char local_refname[256];
    snprintf(local_refname, sizeof(local_refname), "refs/heads/%s", branch_name);
    
    /* Create local branch */
    r = git_reference_create(&local_ref, repo, local_refname,
                            git_object_id(target), 1, NULL);
    if (r < 0)
        goto cleanup;
    
    /* Set HEAD on this branch */
    r = git_repository_set_head(repo, local_refname);

cleanup:
    git_object_free(target);
    git_reference_free(local_ref);
    git_reference_free(remote_ref);
    return r;
}

int check_first(char* string, char* for_check, int len){
  for(int i = 0; i < len; i++){
    if(for_check[i] != string[i]){
      return 0;
    }
  }
  return 1;
}

char* check_for_arg(int argc, char** argv, const char* for_check){
  int len = strlen(for_check);
  for(int i = 0; i < argc; i++){
    if(!check_first(argv[i], for_check, len)){
      continue;
    }
    // Find filename
    int new_len = strlen(argv[i]) - len;
    memmove(argv[i], argv[i] + len, new_len);
    char* filename = argv[i];

    argv[i][new_len] = '\0';
    printf("Find agument %s\n", argv[i]);

    for(int j = i; j < argc - 1; j++){
      argv[j] = argv[j + 1];
    }
    return filename;
      
  }
  return NULL;
}

git_commit * get_last_commit( git_repository * repo )
{
  int rc;
  git_commit * commit = NULL; /* the result */
  git_oid oid_parent_commit;  /* the SHA1 for last commit */

  /* resolve HEAD into a SHA1 */
  rc = git_reference_name_to_id( &oid_parent_commit, repo, "HEAD" );
  if ( rc == 0 )
  {
    /* get the actual commit structure */
    rc = git_commit_lookup( &commit, repo, &oid_parent_commit );
    if ( rc == 0 )
    {
      return commit;
    }
  }
  return NULL;
}


char user_want_commit(char* string){
  const char* c = "commit";
  for(int i = 0; i < strlen(c); i++){
    if(string[i] != c[i])
      return 0; 
  }
  return 1;
}


void print_last_commit_sha(char* path){
  
  git_oid oid;
  git_commit* commit = NULL;
  git_repository *repo = NULL;
  char* oid_str = malloc(GIT_OID_HEXSZ + 1); // GIT_OID_HEXSZ is the full SHA length

  int err = git_repository_open(&repo, path);
  if (err < 0) {
    goto err_exit;
  }
  // Resolve "HEAD" to its commit OID
  err = git_reference_name_to_id(&oid, repo, "HEAD"); 
  
  if (err < 0) {
    goto err_exit;
  }

  git_oid_tostr(oid_str, sizeof(oid_str), &oid);
  printf("%s", oid_str);
  return;

err_exit:
  const git_error *e = git_error_last();
  printf("Error %d/%d: %s\n", err, e->klass, e->message);
  exit(err);
  return;
}

int main(int argc, char **argv) {
    git_libgit2_init();
    git_repository *repo = NULL;

    if(user_want_commit(argv[1])){
      print_last_commit_sha(argv[2]);
      exit(0);

    } 
    
    PAT = check_for_arg(argc, argv, "pat=");
    printf("Pat is %s\n", PAT); 
    if(PAT){
      argc -= 1;
    } else {
      SSH_KEY_FILE = check_for_arg(argc, argv, "ssh_key=");
      if(SSH_KEY_FILE){
        argc -= 1;
      }

      SSH_PUB_FILE = check_for_arg(argc, argv, "pub_key=");
      if(SSH_PUB_FILE){
      argc -= 1;
      }
    }

    printf("Private key filename %s\nPublic key filename %s\n", SSH_KEY_FILE, SSH_PUB_FILE);


    if(argc < 3){
      printf("Usage: gitclone [url] [destination dir] {optional:branch} {optional: ssh_key=/filepath/}\n");
    return 1;
    }

    const char *branch = (argc >= 4) ? argv[3] : NULL;
    git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    fetch_opts.callbacks.transfer_progress = transfer_progress_cb;

    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;

    /* shallow clone */
    fetch_opts.depth = 1;
    fetch_opts.callbacks.transfer_progress = transfer_progress_cb;


    if(PAT){
      printf("Adding pat callback\n");
      fetch_opts.callbacks.credentials = github_pat_cb;
    } else {
      if(SSH_KEY_FILE){
        fetch_opts.callbacks.credentials = cred_acquire_cb;
      } else {
      fetch_opts.callbacks.credentials = cred_agent_cb;
      }

    }

    if (branch) {
        printf("Cloning branch %s\n", branch);
        opts.checkout_branch = branch;
    }

    opts.fetch_opts = fetch_opts;
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
      fetch_opts.depth = 0; // fetch full history
      r = git_remote_fetch(remote, NULL, &fetch_opts, NULL);
      r = fast_forward(repo, branch);
    }

    if (r < 0) {

        const git_error *e = git_error_last();
        fprintf(stderr, "Error: %s\n", e && e->message ? e->message : "unknown\n");
    } else {
      
    }
    
    git_repository_free(repo);
    git_libgit2_shutdown();
    return r;
}
