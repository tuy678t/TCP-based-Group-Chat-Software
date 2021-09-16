-- MySQL dump 10.14  Distrib 5.5.56-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: db1652196
-- ------------------------------------------------------
-- Server version	5.5.56-MariaDB

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES gbk */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `db1652196`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `db1652196` /*!40100 DEFAULT CHARACTER SET gbk */;

USE `db1652196`;

--
-- Table structure for table `filetrans`
--

DROP TABLE IF EXISTS `filetrans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `filetrans` (
  `tid` int(11) NOT NULL AUTO_INCREMENT,
  `sendid` int(11) NOT NULL,
  `recvid` int(11) NOT NULL,
  `allsiz` int(11) NOT NULL,
  `filename` varchar(80) NOT NULL,
  `succsiz` int(11) NOT NULL DEFAULT '0',
  `statue` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`tid`),
  KEY `FILETRANS_SENDID_FK` (`sendid`),
  KEY `FILETRANS_RECVID_FK` (`recvid`),
  CONSTRAINT `FILETRANS_RECVID_FK` FOREIGN KEY (`recvid`) REFERENCES `login` (`uid`),
  CONSTRAINT `FILETRANS_SENDID_FK` FOREIGN KEY (`sendid`) REFERENCES `login` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `filetrans`
--

LOCK TABLES `filetrans` WRITE;
/*!40000 ALTER TABLE `filetrans` DISABLE KEYS */;
/*!40000 ALTER TABLE `filetrans` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `login`
--

DROP TABLE IF EXISTS `login`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `login` (
  `uid` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(50) NOT NULL,
  `password` varchar(50) NOT NULL,
  `first` tinyint(1) NOT NULL,
  `rcnt` int(11) NOT NULL DEFAULT '10',
  `color` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=gbk;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `login`
--

LOCK TABLES `login` WRITE;
/*!40000 ALTER TABLE `login` DISABLE KEYS */;
INSERT INTO `login` VALUES (1,'root','21232f297a57a5a743894a0e4a801fc3',1,10,16007990),(2,'myz','79e262a81dd19d40ae008f74eb59edce',1,10,4149685),(3,'gcj','309825a0951b3cf1f25e27b61cee8243',1,10,38536),(4,'zcl','11e5dfc68422e697563a4253ba360615',1,10,7951688);
/*!40000 ALTER TABLE `login` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `record`
--

DROP TABLE IF EXISTS `record`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `record` (
  `rid` int(11) NOT NULL AUTO_INCREMENT,
  `sendid` int(11) NOT NULL,
  `recvid` int(11) NOT NULL,
  `time` bigint(20) NOT NULL,
  `message` varchar(2048) NOT NULL,
  PRIMARY KEY (`rid`),
  KEY `RECORD_SENDID_FK` (`sendid`),
  KEY `RECORD_RECVID_FK` (`recvid`),
  CONSTRAINT `RECORD_SENDID_FK` FOREIGN KEY (`sendid`) REFERENCES `login` (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=gbk;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `record`
--

LOCK TABLES `record` WRITE;
/*!40000 ALTER TABLE `record` DISABLE KEYS */;
INSERT INTO `record` VALUES (1,1,4,1545436507,'hello'),(2,4,1,1545436532,'hi'),(3,1,4,1545436563,'Bye!'),(4,4,1,1545436586,'Bye!');
/*!40000 ALTER TABLE `record` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-12-23 11:34:53
