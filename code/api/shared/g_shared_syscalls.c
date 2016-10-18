void trap_DropClient( int clientNum, const char *reason, int length ) {
#ifdef API_Q3
	Q_syscall( G_DROP_CLIENT, clientNum, reason );
#endif
#ifdef API_ET
	Q_syscall( G_DROP_CLIENT, clientNum, reason, length );
#endif
}