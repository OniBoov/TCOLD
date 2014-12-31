CREATE TABLE db_mig_script(
	id MEDIUMINT NOT NULL AUTO_INCREMENT,
	job_name VARCHAR(50),
	script_name VARCHAR(200),
	event_date TIMESTAMP,
	status VARCHAR(1),
	reason TEXT,
	PRIMARY KEY (id)
);