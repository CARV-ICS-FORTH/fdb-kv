# Running fdb-hammer with Parallax

## 1. Build fdb-kv

- Read the `README.md` file in the project, and also check the `Dockerfile` to understand how to download the necessary dependencies for the project.
- **IMPORTANT:** You must provide the path to your installed dependencies (like `eckit`, `metkit`, and `eccodes`) using `-DCMAKE_PREFIX_PATH`:

```bash
ecbuild --prefix=~/local \
  -DCMAKE_BUILD_TYPE="Release" \
  -DUSE_PARALLAX=ON \
  -DHAVE_PARALLAXFDB=ON \
  -DCMAKE_PREFIX_PATH="/path/to/your/local/dependencies" \
  /path/to/fdb-kv/source
```

## 2. Build Parallax

Parallax provides two tested branches that support fdb.
There are two Parallax servers:

- One using TCP/IP protocol
- One using InfiniBand (RDMA)

### TCP/IP

- Checkout the branch:
  ```
  git checkout parallax_server_msg_rework
  ```
- Build Parallax with the following flags:
  ```
  -DSEGMENT_SIZE=134217728 -DCUSTOM_KV_MAX_SIZE=4194304
  ```
- Follow the Parallax build instructions if additional guidance is needed.

### Infiniband (RDMA)

- Checkout the branch:

  ```
   git checkout parallax_infiniband
  ```

  For build details, see the README: [Infiniband Parallax Server README](https://carvgit.ics.forth.gr/storage/parallax/-/blob/parallax_infiniband/docs/infiniband_parallax_server.md?ref_type=heads)

## 3. Start the Parallax Server

### TCP/IP

- Run the Parallax server executable on `localhost:8080`.

### InfiniBand

- Run the Parallax server on your system’s configured Infiniband interface address.
- For more details, refer again to the linked README.

## 4. Run the fdb-hammer executable

- Execute the `fdb-hammer` executable with the required parameters.
- **IMPORTANT:** Always set the `nparams` parameter to `4`.
