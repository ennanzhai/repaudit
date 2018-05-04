# RepAudit Project

In this project, we build a novel language framework, 
named RepAudit, that
manages to prevent correlated failure risks before service outages occur, 
by allowing administrators to proactively audit the replication 
deployments of interest. 



In particular, RepAudit consists of three main components: 
1) A declarative domain-specific language, RAL, for cloud administrators 
to write auditing programs expressing diverse auditing tasks; 
2) A high-performance RAL
auditing engine that generates the auditing results by accurately and 
efficiently analyzing the underlying structures of the target replication 
deployments; and
3) An RAL-code generator that can automatically produce 
complex RAL programs
based on easily written specifications.
