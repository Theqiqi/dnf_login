# DNF Taiwan Launcher - Database Topologies & Schema Dictionary

This document defines the data structures, indexing behaviors, and cross-database operational scopes utilized by the server cluster.

The custom launcher relies on two distinct database nodes (`d_taiwan` and `taiwan_login`) managed by the backend engine via non-blocking queries.

## 1. System Topology Overview

To preserve maximum backward compatibility with existing server infrastructure, the authentication lifecycle relies on a dual-schema framework:

```
                      [ Launcher MySQL Client ]
                                  │
         ┌────────────────────────┴────────────────────────┐
         ▼ (Authentication Scope)                          ▼ (Runtime Activation Status)
┌────────────────────────────────┐                ┌────────────────────────────────┐
│ DATABASE: d_taiwan             │                │ DATABASE: taiwan_login         │
├────────────────────────────────┤                ├────────────────────────────────┤
│ TABLE:    accounts             │                │ TABLE:    member_login         │
│ ROLE:     Stores master user   │                │ ROLE:     Whitelists active    │
│           credentials & IDs    │                │           UIDs cleared for play│
└────────────────────────────────┘                └────────────────────────────────┘
```

------

## 2. Data Dictionary Specifications

Database A: `d_taiwan`

- **Purpose**: Primary identification directory handling account creations, secure credential hashing, and global unique identity provisioning.

Table: `accounts`

| Column Name   | Data Type     | Key Type    | Nullable | Default Value    | Functional Role / Engineering Notes                          |
| ------------- | ------------- | ----------- | -------- | ---------------- | ------------------------------------------------------------ |
| `UID`         | `INT(11)`     | **PRIMARY** | NO       | *Auto-Increment* | **Primary Account ID Vector**. Generated uniquely at registration. This field maps directly to the first 4 bytes (`0x00-0x03`) of the launcher runtime packet. |
| `accountname` | `VARCHAR(32)` | **UNIQUE**  | NO       | NULL             | **User Account Identifier**. Strictly filtered by `Database::escapeString` before submission to avoid truncation or injection. |
| `password`    | `VARCHAR(32)` | NONE        | NO       | NULL             | **Encrypted Credential Block**. Stores a 32-character lowercase hexadecimal representation of the password string generated via `MD5(password)`. |

------

Database B: `taiwan_login`

- **Purpose**: Gateway operational telemetry tracking and runtime whitelisting. Accounts must be registered here to clear the server channel gateway check.

Table: `member_login`

| Column Name | Data Type | Key Type    | Nullable | Default Value | Functional Role / Engineering Notes                          |
| ----------- | --------- | ----------- | -------- | ------------- | ------------------------------------------------------------ |
| `m_id`      | `INT(11)` | **PRIMARY** | NO       | NULL          | **Foreign Account Binder**. This value must explicitly match the assigned `UID` integer from `d_taiwan.accounts`. Checked via `Database::loginByUid`. |

------

## 3. Core Operational Queries (SQL Mapping)

To prevent connection context lockouts when migrating between data clusters, all queries are executed across explicitly specified table namespaces.

Pipeline A: User Registration (`Database::registerUser`)

When a new player signs up through the launcher interface, the credentials block is written directly to the primary identity ledger:

``` sql
INSERT INTO d_taiwan.accounts (accountname, password) 
VALUES ('<sanitized_username>', '<lowercase_md5_hash>');
```

Use code with caution.

- **ID Retrieval Hook**: Following successful insertion, the C++ connector captures the transaction response using `mysql_insert_id()` to extract the auto-allocated integer `UID`.

Pipeline B: Credential Authentication (`Database::loginByUsername`)

When logging in by name, the launcher verifies the cross-referenced credentials and pulls the master UID block:

``` sql
SELECT * FROM d_taiwan.accounts 
WHERE accountname='<sanitized_username>' AND password='<lowercase_md5_hash>';
```

Use code with caution.

- **Memory Buffer Row Processing**: The client connector inspects row dimensions. If a match occurs, it extracts index `0` of the matched array stream (`row[0]`) to use as the runtime Account ID.

Pipeline C: Active Whitelist Check (`Database::loginByUid` / `Database::checkUidExists`)

When a player attempts a direct UID bypass, or following a successful username match, the engine must confirm the account is actively registered in the server gateway:

``` sql
SELECT * FROM taiwan_login.member_login 
WHERE m_id=<sanitized_uid_integer>;
```

Use code with caution.

- **Gateway Decisions**: If `mysql_num_rows()` returns `0`, the launcher halts execution and returns `ERR_DB_UID_NOT_WHITELISTED` (`0x020F0006`), indicating that while the account might exist, it hasn't been activated for server play.

------

## 4. Administrative Query Signatures (Original Reference)

The following legacy structural patterns were safely extracted during code disassembly and are preserved here for administrative analytics:

Fetching Maximum Active UID (Last Registered Account)

Used by backend monitors to calculate user expansion variables:

``` sql
SELECT * FROM taiwan_login.member_login ORDER BY m_id DESC LIMIT 0,1;
```

Use code with caution.

Fetching Base Active UID (First Registered Account)

Used to confirm the lower boundaries of the active server index layout:

``` sql
SELECT * FROM taiwan_login.member_login LIMIT 0,1;
```