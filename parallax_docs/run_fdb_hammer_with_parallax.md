# Running fdb-hammer with Parallax

1. **Build fdb-kv**
   - Read the `README.md` file in the project, and also check the `Dockerfile` to understand how to download the necessary dependencies for the project.
   - **IMPORTANT:**
     - Use both of these flags:
       ```
       -DUSE_PARALLAX=ON -DHAVE_PARALLAXFDB=ON
       ```

2. **Build Parallax**
   - Checkout the branch `parallax_server_msg_rework`:
     ```
     git checkout parallax_server_msg_rework
     ```
   - Build Parallax with the following flag enabled:
     ```
     -DSEGMENT_SIZE=134217728 -DCUSTOM_KV_MAX_SIZE=4194304
     ```
   - Follow the Parallax build instructions if additional guidance is needed.

3. **Start the Parallax Server**
   - Run the Parallax server executable on `localhost:8080`.
   - Ensure the server is running before starting `fdb-hammer`.

4. **Run the fdb-hammer executable**
   - Execute the `fdb-hammer` binary with the required parameters.
   - **IMPORTANT:** Always set the `nparams` parameter to `4` when running the executable.
