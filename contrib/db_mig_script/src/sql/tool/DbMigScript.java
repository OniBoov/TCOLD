package sql.tool;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Timestamp;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.EnumMap;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import org.jdom.xpath.XPath;

public class DbMigScript {
	
	private enum Database{
		AUTH,
		WORLD,
		CHARACTER
	}

	private class Configuration{
		private final Map<Database, String> connectionURLs;
		private final List<JobDefinition> jobDefinitions;
		private final Map<String, ExclusionRule> rules;
		
		@SuppressWarnings("unchecked")
		public Configuration() throws InconsistentConfigException, InitializationException{
			this.connectionURLs = new EnumMap<Database, String>(Database.class);
			this.jobDefinitions = new ArrayList<JobDefinition>();
			this.rules = new HashMap<String, ExclusionRule>();
			
			SAXBuilder builder = new SAXBuilder();
			File file = new File("conf.xml");
			
			try{
				Document document = (Document) builder.build(file);
				List<Element> elements = null;
				
				elements = (List<Element>)XPath.selectNodes(document, "Configuration/ExclusionRules/ExclusionRule");
				for(Element element : elements){
					String name = element.getAttributeValue("name");
					ExclusionRuleType type = ExclusionRuleType.valueOf(element.getChildText("Type"));
					
					List<Element> params = (List<Element>)XPath.selectNodes(element, "Params/Param");
					Map<String, String> hm = new HashMap<String, String>();
					for(Element param : params){
						String paramName = param.getAttributeValue("name");
						String paramValue = param.getAttributeValue("value");
						hm.put(paramName, paramValue);
					}
					
					this.rules.put(name, ExclusionRule.create(name, type, hm));
				}
				
				elements = (List<Element>)XPath.selectNodes(document, "Configuration/ConnectionURLs/ConnectionURL");
				for(Element element : elements){
					Database db = Database.valueOf(element.getChildText("Database"));
					String url = element.getChildText("URL");
					this.connectionURLs.put(db, url);
				}
				
				elements = (List<Element>)XPath.selectNodes(document, "Configuration/JobDefinitions/JobDefinition");
				for(Element element : elements){
					String name = element.getAttributeValue("name");
					String path = element.getChildText("Directory");
					File directory = new File(path);
					if(!directory.exists()){
						throw new InconsistentConfigException(directory.getAbsolutePath() + " does not exist");
					}
					
					if(!directory.isDirectory()){
						throw new InconsistentConfigException(directory.getAbsolutePath() + " isn't a directory");
					}
					
					String extension = element.getChildText("Extension");
					boolean recursive = Boolean.valueOf(element.getChildText("Recursive"));
				
					List<Element> rules = (List<Element>)XPath.selectNodes(element, "ExclusionRules/ExclusionRule");
					List<ExclusionRule> ruleList = new ArrayList<ExclusionRule>();
					for(Element rule : rules){
						String ruleName = rule.getAttributeValue("name");
						ExclusionRule ruleObject = this.rules.get(ruleName);
						if(ruleObject == null){
							throw new InconsistentConfigException("Unfound rule name : " + ruleName);
						}
						ruleList.add(ruleObject);
					}
					
					this.jobDefinitions.add(new JobDefinition(name, directory, recursive, extension, ruleList));
				}
				
			} catch (IOException e) {
				throw new InitializationException(e.getMessage(), e);
		  	} catch (Exception e) {
		  		throw new InconsistentConfigException(e.getMessage(), e);
		  	}
		}
		
		public Map<Database, String> getConnectionURLs() {
			return connectionURLs;
		}
		
		public List<JobDefinition> getJobDefinitions() {
			return jobDefinitions;
		}

		@Override
		public String toString() {
			return "Configuration [\n\tconnectionURLs=" + connectionURLs
					+ ", \n\tjobDefinitions=" + jobDefinitions + "\n]";
		}
	}
	
	private class JobDefinition{
		private final String name;
		private final File directory;
		private final boolean recursive;
		private final String extension;
		private final List<ExclusionRule> exclusionRules;
		
		public JobDefinition(String name, File directory, boolean recursive,
				String extension, List<ExclusionRule> exclusionRules) {
			super();
			this.name = name;
			this.directory = directory;
			this.recursive = recursive;
			this.extension = extension;
			this.exclusionRules = exclusionRules;
		}
		public File getDirectory() {
			return directory;
		}
		public boolean isRecursive() {
			return recursive;
		}
		public String getExtension() {
			return extension;
		}
		public String getName() {
			return name;
		}
		public List<ExclusionRule> getExclusionRules() {
			return exclusionRules;
		}
		@Override
		public String toString() {
			return "JobDefinition [name=" + name + ", directory=" + directory
					+ ", recursive=" + recursive + ", extension=" + extension
					+ "]";
		}
	}
	
	private class Context{
		private final Map<Database, Manager> managers;
		
		public Context(Configuration configuration) throws InitializationException{
			managers = new EnumMap<Database, Manager>(Database.class);
			this.init(configuration);
		}
		
		private void init(Configuration configuration) throws InitializationException{
			try {
				Class.forName("com.mysql.jdbc.Driver");
			} catch (ClassNotFoundException e) {
				throw new InitializationException("Can't get MySQL driver", e);
			}
			
			Map<Database, String> connectionURLs = configuration.getConnectionURLs();
			for(Database database : connectionURLs.keySet()){
				managers.put(database, new Manager(connectionURLs.get(database)));
			}
		}
		
		public Manager getManager(Database database){
			return this.managers.get(database);
		}

		public Map<Database, Manager> getManagers() {
			return managers;
		}
	}
	
	private class Manager{
		private final Connection connection;
		private final PreparedStatement insertResult;
		private final PreparedStatement selectResult;
		
		public Manager(String url) throws InitializationException{
			try {
				this.connection = DriverManager.getConnection(url);
				this.connection.setAutoCommit(false);
				this.insertResult = this.connection.prepareStatement("INSERT INTO db_mig_script (job_name, script_name, status, reason) VALUES (?, ?, ?, ?)");
				this.selectResult = this.connection.prepareStatement("SELECT id FROM db_mig_script WHERE status = 'S' AND job_name = ? AND script_name = ?");
			} catch (SQLException e) {
				throw new InitializationException("Can't open connection for " + url, e);
			}
		}
		
		public void execute(String jobName, String scriptName, String sql) throws ExecutionException{

			String lineBreak = "\n";
			
			Statement stm;
			try {
				stm = this.connection.createStatement();
			} catch (SQLException e) {
				throw new ExecutionException(jobName, scriptName, "Unable to create statement");
			}
			
			int lineNum = 0;
			StringBuilder command = new StringBuilder();
			for(String line : sql.split(lineBreak)){
				try{
					if(line.trim().matches("^--.*")){
						continue;
					}
					
					command.append(line);
					if(line.trim().matches(".*;(([ \t]*)?(--.*)?)?$")){
						try {
							stm.execute(command.toString().replaceAll("--.*", ""));
							command = new StringBuilder();
						} catch (SQLException e) {
							throw new ExecutionException(jobName, scriptName, this.getMessageCorrection(e.getMessage(), lineNum));
						}
					}
				}finally{
					lineNum++;
				}
			}
		}
		
		private String getMessageCorrection(String message, int startLine){
			final String regex = "at line \\d+";
			
			Pattern pattern = Pattern.compile(regex);
	        Matcher matcher = pattern.matcher(message);
			if(matcher.find()){
				String str = matcher.group();
				Scanner scanner = new Scanner(str);
				scanner.skip("at line ");
				int messageLine = scanner.nextInt();
				int realLine = startLine + messageLine;
				scanner.close();
				
				return message.replace(str, "at line " + realLine);
			}else{
				return message + (message.endsWith(".")?"":".") + " Query start line: " + startLine;
			}
		}
		
		public void commit() {
			try {
				this.connection.commit();
			} catch (SQLException e) {
				throw new RuntimeException(e.getMessage(), e);
			}
		}
		
		public void rollback() {
			try {
				this.connection.rollback();
			} catch (SQLException e) {
				throw new RuntimeException(e.getMessage(), e);
			}
		}
		
		public void insertResult(ExecutionResult result){
			try {
				this.insertResult.setString(1, result.getJobName());
				this.insertResult.setString(2, result.getScriptName());
				this.insertResult.setString(3, result.getStatus().getCode());
				this.insertResult.setString(4, result.getReason());
				this.insertResult.executeUpdate();
				this.connection.commit();
			} catch (SQLException e) {
				throw new RuntimeException(e.getMessage(), e);
			}
		}
		
		public boolean scriptNeedProcess(String jobName, String scriptName){
			try {
				this.selectResult.setString(1, jobName);
				this.selectResult.setString(2, scriptName);
				return !this.selectResult.executeQuery().next();
			} catch (SQLException e) {
				throw new RuntimeException(e.getMessage(), e);
			}
		}
	}
	
	private class Processor{
		private final Configuration configuration;
		private final Context context;
		
		public Processor() throws InitializationException, InconsistentConfigException{
			this.configuration = new Configuration();
			this.context = new Context(this.configuration);
		}
		
		public void process() {
			
			boolean success = true;
			List<ExecutionResult> results = new ArrayList<ExecutionResult>();
			ExecutionException exception = null;
			
			try{
				for(JobDefinition jobDefinition : configuration.getJobDefinitions()){
					Logger.log("Processor", "Launching job [" + jobDefinition.getName() + "] on " + jobDefinition.getDirectory().getAbsolutePath());
					Job job = new Job(this.context, jobDefinition);
					job.process(results);
				}
			}catch(ExecutionException e){
				success = false;
				exception = e;
			}finally{
				if(success){
					Logger.log("Processor", "Process result : SUCCESS (" + results.size() + " files processed)");
				}else{
					Logger.error("Processor", "Process result : FAIL\nTransaction rollback. Fix the issue and run the process again");
				}
				this.setExecutionOutcome(results, success, exception);
				this.storeResult(results);
				this.writeLog();
			}
		}
		
		private void setExecutionOutcome(List<ExecutionResult> results, boolean success, ExecutionException exception) {
			
			for(Database database : this.context.getManagers().keySet()){
				Manager manager = this.context.getManager(database);
				if(success){
					manager.commit();
				}else{
					manager.rollback();
				}
			}
			
			if(success){
				return;
			}
			for(ExecutionResult result : results){
				if(result.getStatus() == ExecutionStatus.FAIL){
					continue;
				}else{
					result.setStatus(ExecutionStatus.ROLLBACK);
					if(exception != null){
						result.setReason("Rollbacked because of " + exception.getJobName() + "[" + exception.getScriptName() + "]");
					}
				}
			}
		}
		
		private void storeResult(List<ExecutionResult> results){
			for(ExecutionResult result : results){
				Manager manager = this.context.getManager(result.getDatabase());
				if(manager != null){
					manager.insertResult(result);
				}
			}
		}
		
		private void writeLog(){
			File file = new File("log.txt");
			
			try {
				if (!file.exists()) {
					file.createNewFile();
				}
				FileWriter fw = new FileWriter(file.getAbsoluteFile());
				BufferedWriter bw = new BufferedWriter(fw);
			
				bw.write(Logger.getBuffer().toString());
				bw.close();
			} catch (IOException e) {
				throw new RuntimeException();
			}
		}
	}
	
	private class ExecutionResult{
		private final Database database;
		private final String jobName;
		private final String scriptName;
		private ExecutionStatus status;
		private String reason;
		
		public ExecutionResult(Database database, String jobName, String scriptName, ExecutionStatus status){
			this(database, jobName, scriptName, status, null);
		}
		
		public ExecutionResult(Database database, String jobName, String scriptName, ExecutionStatus status, String reason){
			this.database = database;
			this.jobName = jobName;
			this.scriptName = scriptName;
			this.status = status;
			this.reason = reason;
		}

		public ExecutionStatus getStatus() {
			return status;
		}
		public void setStatus(ExecutionStatus status) {
			this.status = status;
		}
		public String getJobName() {
			return jobName;
		}
		public String getScriptName() {
			return scriptName;
		}
		public String getReason() {
			return reason;
		}
		public void setReason(String reason) {
			this.reason = reason;
		}
		public Database getDatabase() {
			return database;
		}
	}
	
	private enum ExecutionStatus{
		SUCCESS("S"),
		FAIL("F"),
		ROLLBACK("R");
		
		private final String code;
		
		ExecutionStatus(String code){
			this.code = code;
		}

		public String getCode() {
			return code;
		}
	}
	
	private class Job{
		private final Context context;
		private final JobDefinition definition;
		
		public Job(Context context, JobDefinition definition){
			this.context = context;
			this.definition = definition;
		}
		
		public void process(List<ExecutionResult> results) throws ExecutionException{
			String jobName = this.definition.getName();
			File directory = this.definition.getDirectory();
	
			List<File> files = this.listFiles(directory, null);
			Collections.sort(files, new Comparator<File>(){
			    public int compare(File f1, File f2){
			        return f1.getAbsolutePath().compareTo(f2.getAbsolutePath());
			    }});
			
			int num = 1;
			for(File file : files){
				
				String name = this.getRelativeName(file, directory);
				Database database = null;
				
				ExclusionRule rule = null;
				for(ExclusionRule r : this.definition.getExclusionRules()){
					if(r.isExcluded(name, file)){
						rule = r;
						break;
					}
				}
				if(rule != null){
					Logger.log("Job", "(" + num + "/" + files.size() + ") Script [" + name + "] is excluded for the rule [" + rule.getName() + "].");
				}else{
					try {
						String content = new String(Files.readAllBytes(Paths.get(file.getAbsolutePath())));
						database = this.findDatabase(file);
						if(database == null){
							String message = "Can't find the database for file : " + file.getAbsolutePath();
							Logger.error("Job", message);
							throw new ExecutionException(jobName, name, message);
						}
						
						Manager manager = this.context.getManager(database);
						if(!manager.scriptNeedProcess(jobName, name)){
							Logger.log("Job", "(" + num + "/" + files.size() + ") Script [" + name + "] on database [" + database.name() + "] already processed.");
						}else{
							Logger.log("Job", "(" + num + "/" + files.size() + ") Starting process of script [" + name + "] on database [" + database.name() + "]");
							manager.execute(jobName, name, content);
							results.add(new ExecutionResult(database, jobName, name, ExecutionStatus.SUCCESS));
						}
					} catch (IOException e) {
						Logger.error("Job", "Script [" + name + "] Error : " + e.getMessage());
						throw new RuntimeException(e.getMessage(), e);
					} catch (ExecutionException e) {
						results.add(new ExecutionResult(database, jobName, name, ExecutionStatus.FAIL, e.getMessage()));
						Logger.error("Job", "Script [" + name + "] Fail : " + e.getMessage());
						throw e;
					}
				}
				
				num++;
			}
		}
		
		private String getRelativeName(File file, File directory){
			String name = null;
			while(file != null && !file.equals(directory)){
				name = file.getName() + (name==null?"":("\\"+name));
				file = file.getParentFile();
			}
			return name;
		}
		
		private List<File> listFiles(File directory, List<File> list){
			if(list == null){
				list = new ArrayList<File>();
			}
			
			for(File file : directory.listFiles()){
				if(file.isFile()){
					if(definition.getExtension() == null){
						list.add(file);
					}else if (file.getName().toUpperCase().endsWith("." + definition.getExtension().toUpperCase())){
						list.add(file);
					}
				}else if(file.isDirectory() && definition.isRecursive()){
					this.listFiles(file, list);
				}
			}
			return list;
		}
		
		private Database findDatabase(File file){
			do{
				file = file.getParentFile();
				if(file == null){
					return null;
				}
				String name = file.getName().toUpperCase();
				try{
					return Database.valueOf(name);
				}catch(Exception e){
					continue;
				}
			}while(true);
		}
	}
	
	private enum ExclusionRuleType{
		FILENAME_TS
	}
	
	private static abstract class ExclusionRule{
		private final String name;
		protected final Map<String, String> params;
		
		public static final String PARAM_INVERT = "invert";
		public static final String PARAM_PATTERN = "pattern";
		public static final String PARAM_FORMAT = "format";
		public static final String PARAM_THRESHOLD = "threshold";
		
		public ExclusionRule(String name, Map<String, String> params){
			this.name = name;
			this.params = params;
		}
		
		public boolean isExcluded(String name, File file){
			boolean value = this.checkIsExcluded(name, file);
			if(params.containsKey(PARAM_INVERT) && Boolean.valueOf(params.get(PARAM_INVERT))){
				value = !value;
			}
			return value;
		}
		
		protected abstract boolean checkIsExcluded(String name, File file);
		
		public static ExclusionRule create(String name, ExclusionRuleType type, Map<String, String> params){
			switch(type){
			case FILENAME_TS:
				return new ExclusionFilenameTS(name, params);
			}
			return null;
		}

		public String getName() {
			return name;
		}
	}
	
	private static class ExclusionFilenameTS extends ExclusionRule{

		public ExclusionFilenameTS(String name, Map<String, String> params) {
			super(name, params);
		}

		@Override
		protected boolean checkIsExcluded(String name, File file) {
			String patternStr = this.params.get(PARAM_PATTERN);
			String format = this.params.get(PARAM_FORMAT);
			String threshold = this.params.get(PARAM_THRESHOLD);
			
			Pattern pattern = Pattern.compile(patternStr);
	        Matcher matcher = pattern.matcher(name);
			if(matcher.find()){
				String str = matcher.group();
				SimpleDateFormat sdf = new SimpleDateFormat(format);
				try {
					Timestamp ts = new Timestamp(sdf.parse(str).getTime());
					Timestamp tsThreshold = Timestamp.valueOf(threshold);
					return ts.before(tsThreshold);
				} catch (ParseException e) {
					throw new RuntimeException(e.getMessage(), e);
				}
			}else{
				return false;
			}
		}
	}
	
	private class InitializationException extends Exception{
		private static final long serialVersionUID = 8472594579138343608L;

		public InitializationException(String message, Throwable cause) {
			super(message, cause);
		}
	}
	
	private class InconsistentConfigException extends Exception{
		private static final long serialVersionUID = 8472594579138343608L;

		public InconsistentConfigException(String message) {
			super(message);
		}
		
		public InconsistentConfigException(String message, Throwable cause) {
			super(message, cause);
		}
	}
	
	private static class Logger{
		private static StringBuilder buffer = new StringBuilder();
		
		public static void log(String processName, String message){
			write("LOG", processName, message);
		}
		public static void error(String processName, String message){
			write("ERROR", processName, message);
		}
		private static void write(String status, String processName, String message){
			String msg = "[" + status + "] [" + processName + "] " + message;
			System.out.println(msg);
			buffer.append(msg+"\r\n");
		}
		public static StringBuilder getBuffer() {
			return buffer;
		}
	}
	
	private class ExecutionException extends Exception{
		private static final long serialVersionUID = 3786833530239023184L;
		private String jobName;
		private String scriptName;
		
		public ExecutionException(String jobName, String scriptName, String message){
			super(message);
			this.jobName = jobName;
			this.scriptName = scriptName;
		}
		
		public String getJobName() {
			return jobName;
		}
		
		public String getScriptName() {
			return scriptName;
		}
	}
	
	public static void main(String[] args) throws InitializationException, InconsistentConfigException{
	
		Processor p = new DbMigScript().new Processor();
		p.process();
	}
}
