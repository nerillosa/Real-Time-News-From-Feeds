CREATE TABLE `agency` (
  `fullname` varchar(25) DEFAULT NULL,
  `shortname` varchar(25) NOT NULL DEFAULT '',
  `logo` varchar(25) DEFAULT NULL,
  PRIMARY KEY (`shortname`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `news_type` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` varchar(25) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE news (
  agency varchar(25) NOT NULL,
  url varchar(512) DEFAULT NULL,
  title varchar(512) NOT NULL,
  pubdate datetime NOT NULL,
  news_type int(11) NOT NULL,
  html text NOT NULL,
  img varchar(512) DEFAULT NULL,
  create_date datetime DEFAULT NULL,
  PRIMARY KEY (agency,news_type,img,html(255)),
  KEY news_ibfk_1 (news_type),
  CONSTRAINT news_ibfk_1 FOREIGN KEY (news_type) REFERENCES news_type (id),
  CONSTRAINT news_ibfk_2 FOREIGN KEY (agency) REFERENCES agency (shortname)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

insert into agency (fullname,shortname,logo) values ('USA TODAY','US TODAY','USATODAY.png');
insert into agency (fullname,shortname,logo) values ('WASHINGTON POST','WSH POST','WPST.png');
insert into agency (fullname,shortname,logo) values ('NEW YORK TIMES','NY TIMES','NYT.png');
insert into agency (fullname,shortname,logo) values ('CNBC','CNBC','CNBC.png');
insert into agency (fullname,shortname,logo) values ('FOX NEWS','FOX NEWS','FOX.png');
insert into agency (fullname,shortname,logo) values ('REUTERS','REUTERS','REUTERS.png');
insert into agency (fullname,shortname,logo) values ('ABC NEWS','ABC NEWS','ABC.png');
insert into agency (fullname,shortname,logo) values ('CNN','CNN','CNN.png');

insert into news_type(type) values('POLITICS');
insert into news_type(type) values('SCIENCE');
insert into news_type(type) values('WORLD');
insert into news_type(type) values('SPORTS');
insert into news_type(type) values('ENTERTAINMENT');
insert into news_type(type) values('HEALTH');
insert into news_type(type) values('USA')

